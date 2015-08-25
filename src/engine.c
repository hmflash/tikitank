#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "common.h"
#include "engine.h"
#include "web.h"
#include "effects/effects.h"

// conversions:
// 3 in/tick
// 32 px/m
// 3 in = 0.0254 m
// 3 in * 0.0254 m = 0.0762 m
// 0.0762 m/tick
// 32 px/m * 0.0762 m/tick = 2.4384 px/tick

#define   INCH_PER_TICK     2.75
#define  METER_PER_INCH     0.0254
#define    LED_PER_METER   32
#define    INC_PER_LED    256
#define    SEC_PER_MIN     60

#define    LED_PER_TICK   (LED_PER_METER * INCH_PER_TICK * METER_PER_INCH)
#define    INC_PER_TICK   (INC_PER_LED * LED_PER_TICK)

struct engine {
	struct pal*        pal;
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	volatile int       exit;
	unsigned int       last_tick;
	unsigned int       idle_frames;
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

	LOG(("led/tick = %g\n", LED_PER_TICK));
	LOG(("inc/tick = %g\n", INC_PER_TICK));

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

void select_next_screen_saver(struct channel* c) {
	int i;
	int j;

	for (i = 0, j = c->idle + 1; i < c->num_effects; i++, j++) {
		struct effect* effect = c->effects[j % c->num_effects];
		if (effect->screen_saver) {
			LOG(("next screen saver: %s\n", effect->name));
			c->idle = j % c->num_effects;
			return;
		}
	}

	LOG(("no screen saver set, using 1st effect\n"));
	c->idle = 0;
}

struct effect* get_effect(struct channel* c) {
	int idle_interval = settings.idle_interval * FRAME_PER_SEC * SEC_PER_MIN;
	if (settings.screen_saver_toggle && 
		c != &channel_panels && 
		eng.idle_frames >= idle_interval) {
		if (eng.idle_frames % idle_interval == 0) {
			select_next_screen_saver(c);
		}
		return c->effects[c->idle < c->num_effects ? c->idle : 0];
	}
	return c->effects[c->active < c->num_effects ? c->active : 0];
}

int engine_run() {
	int ret;
	int framenum = 0;
	int shift = 0;
	double shift_last = 0;
	struct timespec tv;

	pthread_mutex_lock(&eng.mutex);

	pal_clock_gettime(&tv);
	
	eng.last_tick = *eng.pal->enc_ticks;

	while (!eng.exit) {
		double shift_inc;
		double alpha = settings.alpha / 100.0;
		unsigned int sensor_ticks = *eng.pal->enc_ticks;
		int dt = (int)sensor_ticks - (int)eng.last_tick;
		char* treads_buf = eng.pal->treads_buf;
		char* barrel_buf = eng.pal->barrel_buf;

		++framenum;
		shift_inc = settings.manual_toggle ? settings.manual_tick : dt * INC_PER_TICK;
		shift_inc = alpha * shift_inc + (1.0 - alpha) * shift_last;
		shift_last = shift_inc;
		shift += shift_inc;

		if (shift_inc >= 1.0) {
			eng.idle_frames = 0;
		} else {
			eng.idle_frames++;
		}

		struct render_args treads_args = {
			.effect          = get_effect(&channel_treads),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = treads_buf,
			.framelen        = NUM_TREADS,
		};

		struct render_args barrel_args = {
			.effect          = get_effect(&channel_barrel),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = barrel_buf,
			.framelen        = NUM_BARREL,
		};

		struct render_args panels_args = {
			.effect          = get_effect(&channel_panels),
			.shift_quotient  = shift / 0xff,
			.shift_remainder = shift % 0xff,
			.framenum        = framenum,
			.framebuf        = eng.pal->panels_buf,
			.framelen        = NUM_PANELS,
		};

		eng.last_tick = sensor_ticks;

		DEBUG_LOG(("Timer: %u, Raw: %u, Min: %u, Max: %u, Ticks: %u, Delta: %d, Speed: %u\n",
		           *eng.pal->enc_timer,
		           *eng.pal->enc_raw,
		           *eng.pal->enc_min,
		           *eng.pal->enc_max,
		           *eng.pal->enc_ticks,
		           dt,
			   *eng.pal->enc_speed));

		treads_args.effect->render(&treads_args);
		barrel_args.effect->render(&barrel_args);
		panels_args.effect->render(&panels_args);

		ret = pthread_cond_timedwait(&eng.cond, &eng.mutex, &tv);
		if (ret != ETIMEDOUT) {
			LOG(("ret: (%d) %s, errno: (%d) %s\n", ret, strerror(ret), errno, strerror(errno)));
		}

		pal_treads_write();
		pal_barrel_write();
		pal_panels_write();

		web_treads_render(treads_buf, NUM_TREADS);
		web_barrel_render(barrel_buf, NUM_BARREL);

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
