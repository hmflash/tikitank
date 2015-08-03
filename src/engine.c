#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "common.h"
#include "engine.h"

#define NUM_TREADS (3*32*5*3)

struct engine {
	struct pal*        pal;
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	volatile int       exit;
	char               framebuf[NUM_TREADS + (5*3)];
};

static 
struct engine eng;

static 
void signal_handler(int signum)
{
	eng.exit = 1;
}

struct engine* engine_init(struct pal* pal) {
	eng.pal = pal;
	eng.exit = 0;

	pthread_mutex_init(&eng.mutex, NULL);
	pthread_condattr_init(&eng.condattr);
	pthread_condattr_setclock(&eng.condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&eng.cond, &eng.condattr);
	pthread_mutex_lock(&eng.mutex);

	memset(eng.framebuf, 0, sizeof(eng.framebuf));

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	return 0;
}

int engine_destroy() {
	pthread_mutex_destroy(&eng.mutex);
	pthread_cond_destroy(&eng.cond);
	pthread_condattr_destroy(&eng.condattr);
	return 0;
}

int engine_run() {
	int ret;
	int i;
	int framenum = 0;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	while (!eng.exit) {
		for (i = 0; i < NUM_TREADS; i += 3) {
			eng.framebuf[i+0] = 0x80 | fastled_rainbow[(i + framenum) % 256][1] >> 1;
			eng.framebuf[i+1] = 0x80 | fastled_rainbow[(i + framenum) % 256][0] >> 1;
			eng.framebuf[i+2] = 0x80 | fastled_rainbow[(i + framenum) % 256][2] >> 1;
		}

		++framenum;

		ret = pthread_cond_timedwait(&eng.cond, &eng.mutex, &tv);
		if (ret != ETIMEDOUT) {
			LOG(("ret: (%d) %s, errno: (%d) %s\n", ret, strerror(ret), errno, strerror(errno)));
		}

		pal_write_treads(eng.pal, eng.framebuf, sizeof(eng.framebuf));

		tv.tv_nsec += 20000000; // advance by 20ms
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	LOG(("Turning off LEDs\n"));

	memset(eng.framebuf, 1, sizeof(eng.framebuf));
	pal_write_treads(eng.pal, eng.framebuf, sizeof(eng.framebuf));

	return 0;
}
