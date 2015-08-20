#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "effects/effects.h"

// 1 mph = 0.44704 m/s
// 0.0762 m/tick
// 50 frame/s
// 0.44704 / 50 = 0.0089408
// 0.0762 / 0.0089408 = 8.522

struct osx_pal
{
	struct pal            p;

	pthread_t             thread;
	volatile int          exit;

	int                   mod;
	int                   inc;

	char                  treads_buf[NUM_TREADS];
	char                  barrel_buf[NUM_BARREL];
	char                  panels_buf[NUM_PANELS];

	volatile unsigned int enc_timer; // Number of ADC reads
	volatile unsigned int enc_raw;   // Raw ADC value
	volatile unsigned int enc_min;   // Min value for current half-tick
	volatile unsigned int enc_max;   // Max value for current half-tick
	volatile unsigned int enc_ticks; // Number of encoder ticks
	volatile unsigned int enc_speed; // Width of last encoder tick
};

static
struct osx_pal pal;

static
void* pal_thread(void* arg) {	
	LOG(("pal> thread started\n"));

	while (!pal.exit) {
		int rc;
		FILE* pipe;

		pipe = fopen("/tmp/tikitank", "r");
		if (pipe == NULL) {
			LOG(("pal> fopen() failed: (%d) %s\n", errno, strerror(errno)));
			break;
		}

		rc = fscanf(pipe, "%d %d\n", &pal.mod, &pal.inc);
		if (rc < 0) {
			if (ferror(pipe)) {
				LOG(("pal> fscanf() failed: (%d) %s\n", errno, strerror(errno)));
			}
		}

		if (pal.exit) {
			break;
		}

		if (rc == 2) {
			LOG(("mod: %d, inc: %d\n", pal.mod, pal.inc));
		} else {
			LOG(("Expecting [mod inc]\n"));
		}

		fclose(pipe);
	}

	return NULL;
}

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay, unsigned int ema_pow) {
	int rc;

	memset(&pal, 0, sizeof(pal));
	pal.enc_ticks    = -10;

	pal.p.treads_buf = pal.treads_buf;
	pal.p.barrel_buf = pal.barrel_buf;
	pal.p.panels_buf = pal.panels_buf;
	pal.p.enc_timer  = &pal.enc_timer;
	pal.p.enc_raw    = &pal.enc_raw;
	pal.p.enc_min    = &pal.enc_min;
	pal.p.enc_max    = &pal.enc_max;
	pal.p.enc_ticks  = &pal.enc_ticks;
	pal.p.enc_speed  = &pal.enc_speed;

	rc = mkfifo("/tmp/tikitank", S_IRUSR | S_IWUSR);
	if (rc < 0) {
		LOG(("pal> mkfifo() failed: (%d) %s\n", rc, strerror(rc)));
	}

	pthread_create(&pal.thread, NULL, pal_thread, NULL);

	return &pal.p;
}

void pal_treads_write() {
	static int framenum = 0;

	framenum++;
	if (pal.mod && framenum % pal.mod == 0)
		pal.enc_ticks += pal.inc;
}

void pal_barrel_write() {
}

void pal_panels_write() {
}

void pal_destroy() {
	FILE* pipe;

	pal.exit = 1;

	pipe = fopen("/tmp/tikitank", "w");
	fprintf(pipe, "\n");
	fclose(pipe);

	if (pal.thread) {
		pthread_join(pal.thread, NULL);
	}

	unlink("/tmp/tikitank");
}

int pal_clock_gettime(struct timespec* ts) {
	int rc;
	struct timeval tv = {0, 0};
	
	rc = gettimeofday(&tv, NULL);
	ts->tv_sec  = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;

	return rc;
}
