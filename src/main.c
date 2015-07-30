#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <prussdrv.h>

#include "common.h"

void debug_log(const char const* fmt, ...) {
	va_list arg;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	printf("%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

struct renderer {
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	int                exit;
};

int renderer_init(struct renderer* r) {
	pthread_mutex_init(&r->mutex, NULL);
	pthread_condattr_init(&r->condattr);
	pthread_condattr_setclock(&r->condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&r->cond, &r->condattr);
	pthread_mutex_lock(&r->mutex);
	return 0;
}

int renderer_destroy(struct renderer* r) {
	pthread_mutex_destroy(&r->mutex);
	pthread_cond_destroy(&r->cond);
	pthread_condattr_destroy(&r->condattr);
	return 0;
}

int renderer_run(struct renderer* r) {
	int ret;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	while (!r->exit) {
		ret = pthread_cond_timedwait(&r->cond, &r->mutex, &tv);

		LOG(("Ret: %d, Errno: %d\n", ret, errno));

		tv.tv_nsec += 20000000;
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	return 0;
}

static struct renderer r;

static void signal_handler(int signum)
{
	r.exit = 1;
}

int main(int argc, char** argv) {
	renderer_init(&r);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	LOG(("Running engine\n"));

	renderer_run(&r);
	renderer_destroy(&r);

	LOG(("Exited gracefully\n"));

	return 0;
}

