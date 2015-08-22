#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"
#include "engine.h"
#include "web.h"

int verbosity = 0;

FILE* fout = NULL;

void debug_log(const char* fmt, ...) {
	va_list arg;
	struct timespec tv;

	pal_clock_gettime(&tv);

	fprintf(fout, "%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

	va_start(arg, fmt);
	vfprintf(fout, fmt, arg);
	va_end(arg);

	fflush(fout);
}

static
void usage(int rc) {
	printf("usage: tikitank [-hvd][-p PORT][-l LOGPATH]\n");
	exit(rc);	
}

int main(int argc, char** argv) {
	int rc;
	int bg = 0;
	const char* port = "80";
	const char* logpath = NULL;
	struct pal* p;
	struct engine* eng;

	while ((rc = getopt(argc, argv, "hdvp:l:")) != -1) {
		switch (rc) {
		case 'p':
			port = optarg;
			break;
		case 'v':
			++verbosity;
			break;
		case 'h':
			usage(0);
			break;
		case 'd':
			bg = 1;
			break;
		case 'l':
			logpath = optarg;
			break;
		default:
			usage(2);
			break;
		}
	}

	if (logpath) {
		fout = fopen(logpath, "w");
	}

	if (!fout) {
		fout = stdout;
	}

	if (bg) {
		rc = daemon(1, 0);
		if (rc < 0) {
			LOG(("daemon() failed: (%d) %s\n", errno, strerror(errno)));
			goto fail;
		}
	}

	// enc_thresh, enc_delay, ema_pow
	p = pal_init(1000, 1, 2);
	if (!p) {
		LOG(("pal could not initialize\n"));
		rc = -1;
		goto fail;
	}

	eng = engine_init(p);
	rc = web_init(eng, port);
	if (rc < 0) {
		LOG(("Web server failed to start: (%d) %s\n", rc, strerror(rc)));
		goto fail;
	}

	settings_load();

	LOG(("Running engine\n"));

	web_run();
	engine_run();

	web_destroy();
	engine_destroy();
	pal_destroy();

	LOG(("Exited gracefully\n"));

fail:
	fclose(fout);

	return rc;
}
