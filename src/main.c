#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

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

#define NUM_TREADS (3*32*5*3)

struct renderer {
	pthread_mutex_t    mutex;
	pthread_condattr_t condattr;
	pthread_cond_t     cond;
	int                exit;
	char               framebuf[NUM_TREADS + (5*3)];
};

static struct renderer r;

static struct pal p;

int renderer_init(struct renderer* r) {
	pthread_mutex_init(&r->mutex, NULL);
	pthread_condattr_init(&r->condattr);
	pthread_condattr_setclock(&r->condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&r->cond, &r->condattr);
	pthread_mutex_lock(&r->mutex);

	memset(r->framebuf, 0, sizeof(r->framebuf));

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
	int i;
	int framenum = 0;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	while (!r->exit) {
		for (i = 0; i < NUM_TREADS; i += 3) {
			r->framebuf[i+0] = 0x80 | fastled_rainbow[(i + framenum) % 256][1] >> 1;
			r->framebuf[i+1] = 0x80 | fastled_rainbow[(i + framenum) % 256][0] >> 1;
			r->framebuf[i+2] = 0x80 | fastled_rainbow[(i + framenum) % 256][2] >> 1;
		}

		++framenum;

		ret = pthread_cond_timedwait(&r->cond, &r->mutex, &tv);

		if (ret != ETIMEDOUT)
			LOG(("Ret: %d, Errno: %d\n", ret, errno));

		pal_write_treads(&p, r->framebuf, sizeof(r->framebuf));

		tv.tv_nsec += 2000000;
		tv.tv_sec += tv.tv_nsec / 1000000000;
		tv.tv_nsec = tv.tv_nsec % 1000000000;
	}

	LOG(("Turning off LEDs\n"));

	memset(r->framebuf, 1, sizeof(r->framebuf));
	pal_write_treads(&p, r->framebuf, sizeof(r->framebuf));

	return 0;
}

static 
void signal_handler(int signum) {
	r.exit = 1;
}

int main(int argc, char** argv) {
	if (pal_init(&p))
		return -1;

	renderer_init(&r);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	LOG(("Running engine\n"));

	renderer_run(&r);

	renderer_destroy(&r);
	pal_destroy(&p);

	LOG(("Exited gracefully\n"));

	return 0;
}

