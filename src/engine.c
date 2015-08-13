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

	int                treads_active;
	char               treads_buf[NUM_TREADS + (5*3)];

	int                barrel_active;
	char               barrel_buf[NUM_BARREL + (5*3)];

	int                panels_active;
	char               panels_buf[NUM_PANELS * 3];
};

static 
struct engine eng;

static 
void signal_handler(int signum)
{
	eng.exit = 1;
}

struct engine* engine_init(struct pal* pal) {
	memset(&eng, 0, sizeof(eng));
	eng.pal = pal;
	eng.treads_active = 1;

	pthread_mutex_init(&eng.mutex, NULL);
	pthread_condattr_init(&eng.condattr);
#ifdef Linux
	pthread_condattr_setclock(&eng.condattr, CLOCK_MONOTONIC);
#endif
	pthread_cond_init(&eng.cond, &eng.condattr);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	LOG(("%d treads effects\n", effects_treads_num));
	LOG(("%d barrel effects\n", effects_barrel_num));
	LOG(("%d panels effects\n", effects_panels_num));

	return &eng;
}

int engine_destroy() {
	pthread_mutex_destroy(&eng.mutex);
	pthread_cond_destroy(&eng.cond);
	pthread_condattr_destroy(&eng.condattr);
	return 0;
}

struct effect* get_active(struct effect** effects, int active, int max) {
	return effects[active < max ? active : 0];
}

int engine_run() {
	int ret;
	int framenum = 0;
	int shift = 0;
	struct timespec tv;

	pthread_mutex_lock(&eng.mutex);

	pal_clock_gettime(&tv);

	while (!eng.exit) {
		struct effect* treads;
		struct effect* barrel;
		struct effect* panels;

		treads = get_active(effects_treads, eng.treads_active, effects_treads_num);
		barrel = get_active(effects_barrel, eng.barrel_active, effects_barrel_num);
		panels = get_active(effects_panels, eng.panels_active, effects_panels_num);

		treads->render(treads, shift, framenum, eng.treads_buf, NUM_TREADS);
		barrel->render(barrel, shift, framenum, eng.barrel_buf, NUM_BARREL);
		panels->render(panels, shift, framenum, eng.panels_buf, sizeof(eng.panels_buf));

		++framenum;

		if (framenum % 4 == 0) {
			++shift;
		}

		ret = pthread_cond_timedwait(&eng.cond, &eng.mutex, &tv);
		if (ret != ETIMEDOUT) {
			LOG(("ret: (%d) %s, errno: (%d) %s\n", ret, strerror(ret), errno, strerror(errno)));
		}

		DEBUG_LOG(("Timer: %u, Raw: %u, Min: %u, Max: %u, Ticks: %u\n",
		     *eng.pal->enc_timer,
		     *eng.pal->enc_raw,
		     *eng.pal->enc_min,
		     *eng.pal->enc_max,
		     *eng.pal->enc_ticks));

		pal_treads_write(eng.treads_buf, sizeof(eng.treads_buf));
		pal_barrel_write(eng.barrel_buf, sizeof(eng.barrel_buf));
		pal_panels_write(eng.panels_buf, sizeof(eng.panels_buf));

		web_treads_render(eng.treads_buf, NUM_TREADS);

		tv.tv_nsec += 20000000; // advance by 20ms
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	LOG(("Turning off LEDs\n"));

	memset(eng.treads_buf, 0x80, NUM_TREADS);
	pal_treads_write(eng.treads_buf, sizeof(eng.treads_buf));

	return 0;
}
