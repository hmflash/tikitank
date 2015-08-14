#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "common.h"
#include "engine.h"
#include "web.h"
#include "effects/effects.h"

struct engine {
	struct pal*        pal;
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	volatile int       exit;
};

static 
struct engine eng;

static 
void signal_handler(int signum) {
	eng.exit = 1;
}

struct engine* engine_init(struct pal* pal) {
	memset(&eng, 0, sizeof(eng));
	eng.pal = pal;

	pthread_mutex_init(&eng.mutex, NULL);
	pthread_condattr_init(&eng.condattr);
#ifdef Linux
	pthread_condattr_setclock(&eng.condattr, CLOCK_MONOTONIC);
#endif
	pthread_cond_init(&eng.cond, &eng.condattr);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	LOG(("%d treads effects\n", channel_treads.num_effects));
	LOG(("%d barrel effects\n", channel_barrel.num_effects));
	LOG(("%d panels effects\n", channel_panels.num_effects));

	return &eng;
}

int engine_destroy() {
	pthread_mutex_destroy(&eng.mutex);
	pthread_cond_destroy(&eng.cond);
	pthread_condattr_destroy(&eng.condattr);
	return 0;
}

struct effect* get_active(struct channel* c) {
	return c->effects[c->active < c->num_effects ? c->active : 0];
}

int engine_run() {
	int ret;
	int framenum = 0;
	int shift = 0;
	struct timespec tv;

	pthread_mutex_lock(&eng.mutex);

	pal_clock_gettime(&tv);

	while (!eng.exit) {
		char* treads_buf = eng.pal->treads_buf;
		struct render_args treads_args = {
			.effect          = get_active(&channel_treads),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = treads_buf,
			.framelen        = NUM_TREADS,
		};

		struct render_args barrel_args = {
			.effect          = get_active(&channel_barrel),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = eng.pal->barrel_buf,
			.framelen        = NUM_BARREL,
		};

		struct render_args panels_args = {
			.effect          = get_active(&channel_panels),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = eng.pal->panels_buf,
			.framelen        = NUM_PANELS,
		};

		DEBUG_LOG(("Timer: %u, Raw: %u, Min: %u, Max: %u, Ticks: %u\n",
		           *eng.pal->enc_timer,
		           *eng.pal->enc_raw,
		           *eng.pal->enc_min,
		           *eng.pal->enc_max,
		           *eng.pal->enc_ticks));

		treads_args.effect->render(&treads_args);
		barrel_args.effect->render(&barrel_args);
		panels_args.effect->render(&panels_args);

		++framenum;
		shift += settings.manual_tick;

		ret = pthread_cond_timedwait(&eng.cond, &eng.mutex, &tv);
		if (ret != ETIMEDOUT) {
			LOG(("ret: (%d) %s, errno: (%d) %s\n", ret, strerror(ret), errno, strerror(errno)));
		}

		pal_treads_write();
		pal_barrel_write();
		pal_panels_write();

		web_treads_render(treads_buf, NUM_TREADS);

		tv.tv_nsec += 20000000; // advance by 20ms
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	return 0;
}

void engine_lock() {
	pthread_mutex_lock(&eng.mutex);
}

void engine_unlock() {
	pthread_mutex_unlock(&eng.mutex);
}
