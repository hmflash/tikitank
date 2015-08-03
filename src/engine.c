#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "common.h"
#include "engine.h"

struct engine {
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	volatile int       exit;
};

static 
struct engine eng;

static 
void signal_handler(int signum)
{
	eng.exit = 1;
}

struct engine* engine_init() {
	eng.exit = 0;

	pthread_mutex_init(&eng.mutex, NULL);
	pthread_condattr_init(&eng.condattr);
	pthread_condattr_setclock(&eng.condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&eng.cond, &eng.condattr);
	pthread_mutex_lock(&eng.mutex);

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
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	while (!eng.exit) {
		ret = pthread_cond_timedwait(&eng.cond, &eng.mutex, &tv);
		if (ret != ETIMEDOUT) {
			LOG(("ret: (%d) %s, errno: (%d) %s\n", ret, strerror(ret), errno, strerror(errno)));
		}

		tv.tv_nsec += 20000000; // advance by 20ms
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	return 0;
}
