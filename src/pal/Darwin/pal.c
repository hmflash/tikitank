#include <string.h>
#include <sys/time.h>

#include "common.h"
#include "effects/effects.h"

struct osx_pal
{
	struct pal p;

	char treads_buf[NUM_TREADS];
	char barrel_buf[NUM_BARREL];
	char panels_buf[NUM_PANELS];
};

static
struct osx_pal pal;

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay) {
	memset(&pal, 0, sizeof(pal));

	pal.p.treads_buf = pal.treads_buf;
	pal.p.barrel_buf = pal.barrel_buf;
	pal.p.panels_buf = pal.panels_buf;

	return &pal.p;
}

void pal_treads_write() {
}

void pal_barrel_write() {
}

void pal_panels_write() {
}

void pal_destroy() {
}

int pal_clock_gettime(struct timespec* ts) {
	int rc;
	struct timeval tv = {0, 0};
	
	rc = gettimeofday(&tv, NULL);
	ts->tv_sec  = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;

	return rc;
}
