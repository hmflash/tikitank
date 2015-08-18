#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "renderer.h"

static
void* renderer_run(void* arg) {
	struct renderer* r = (struct renderer*)arg;
	int ret;

	DEBUG_LOG(("%s renderer thread started\n", r->name));

	// Tell main thread we have started
	pthread_cond_signal(&r->cond);
	pthread_mutex_lock(&r->mutex);

	while (1) {
		ret = pthread_cond_wait(&r->cond, &r->mutex);
		if (ret != 0) {
			LOG(("%s thread, ret: (%d) %s, errno: (%d) %s\n",
			     r->name, ret, strerror(ret), errno, strerror(errno)));
		}

		if (r->exit) {
			pthread_mutex_unlock(&r->mutex);
			DEBUG_LOG(("%s renderer thread exited\n", r->name));
			return NULL;
		}

		TRACE_LOG(("%s write begin\n", r->name));

		// Thread always writes buf2
		r->writefn(r->fd, r->buf2, r->buflen);

		TRACE_LOG(("%s write end\n", r->name));
	}
}

int renderer_init(struct renderer* r,
                  const char* name,
                  int fd,
                  char* buf1,
                  char* buf2,
                  size_t buflen,
                  int (*writefn)(int, const char*, size_t),
                  char** start) {
	memset(r, 0, sizeof(*r));

	r->name = name;
	r->fd = fd;
	r->buf1 = buf1;
	r->buf2 = buf2;
	r->buflen = buflen;
	r->writefn = writefn;

	pthread_mutex_init(&r->mutex, NULL);
	pthread_cond_init(&r->cond, NULL);

	// Ensure rendering thread is started before returning
	pthread_mutex_lock(&r->mutex);
	pthread_create(&r->thread, NULL, renderer_run, r);
	pthread_cond_wait(&r->cond, &r->mutex);
	pthread_mutex_unlock(&r->mutex);

	// User always writes into buf1
	*start = buf1;

	return 0;
}

int renderer_swap(struct renderer* r, char** buf) {
	int ret = pthread_mutex_trylock(&r->mutex);

	if (!ret) {
		*buf = r->buf2;
		r->buf2 = r->buf1;
		r->buf1 = *buf;

		pthread_mutex_unlock(&r->mutex);
		pthread_cond_signal(&r->cond);
	}

	return ret;
}

void renderer_destroy(struct renderer* r) {
	if (r->thread == 0)
		return;

	// Ensure we have completed all writes
	// before stopping rendering thread
	pthread_mutex_lock(&r->mutex);

	r->exit = 1;

	pthread_mutex_unlock(&r->mutex);
	pthread_cond_signal(&r->cond);

	pthread_join(r->thread, NULL);
	pthread_mutex_destroy(&r->mutex);
	pthread_cond_destroy(&r->cond);
}
