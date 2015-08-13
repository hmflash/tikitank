#include <string.h>
#include <sys/time.h>

#include "common.h"

static
struct pal pal;

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay) {
	memset(&pal, 0, sizeof(pal));
	return &pal;
}

void pal_treads_write() {
}

void pal_barrel_write() {
}

void pal_panels_write() {
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
