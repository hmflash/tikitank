#include <string.h>
#include <sys/time.h>

#include "common.h"
#include "effects/effects.h"

// 1 mph = 0.44704 m/s
// 0.0762 m/tick
// 50 frame/s
// 0.44704 / 50 = 0.0089408
// 0.0762 / 0.0089408 = 8.522

struct osx_pal
{
	struct pal p;

	char treads_buf[NUM_TREADS];
	char barrel_buf[NUM_BARREL];
	char panels_buf[NUM_PANELS];

	volatile unsigned int enc_timer; // Number of ADC reads
	volatile unsigned int enc_raw;   // Raw ADC value
	volatile unsigned int enc_min;   // Min value for current half-tick
	volatile unsigned int enc_max;   // Max value for current half-tick
	volatile unsigned int enc_ticks; // Number of encoder ticks
	volatile unsigned int enc_speed; // Width of last encoder tick
};

static
struct osx_pal pal;

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay) {
	memset(&pal, 0, sizeof(pal));
	pal.enc_ticks = -10;

	pal.p.treads_buf = pal.treads_buf;
	pal.p.barrel_buf = pal.barrel_buf;
	pal.p.panels_buf = pal.panels_buf;
	pal.p.enc_timer  = &pal.enc_timer;
	pal.p.enc_raw    = &pal.enc_raw;
	pal.p.enc_min    = &pal.enc_min;
	pal.p.enc_max    = &pal.enc_max;
	pal.p.enc_ticks  = &pal.enc_ticks;
	pal.p.enc_speed  = &pal.enc_speed;

	return &pal.p;
}

void pal_treads_write() {
	static int framenum = 0;

	framenum++;
	if (framenum % 8 == 0)
		pal.enc_ticks++;
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
