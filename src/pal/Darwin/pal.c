#include <string.h>
#include <sys/time.h>

#include "common.h"

static
struct pal pal;

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay) {
	memset(&pal, 0, sizeof(pal));
	pal.fd_treads = -1;
	pal.fd_barrel = -1;
	pal.fd_panels = -1;
	return &pal;
}

int pal_treads_write(struct pal* p, const char* buf, size_t len) {
	return len;
}

int pal_barrel_write(struct pal* p, const char* buf, size_t len) {
	return len;
}

int pal_panels_write(struct pal* p, const char* buf, size_t len) {
	return len;
}

void pal_destroy(struct pal* p) {
}

int pal_clock_gettime(struct timespec* ts) {
	int rc;
	struct timeval tv = {0, 0};
	
	rc = gettimeofday(&tv, NULL);
	ts->tv_sec  = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;

	return rc;
}
