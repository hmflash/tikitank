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

void debug_log(const char* fmt, ...) {
	va_list arg;
	struct timespec tv;

	pal_clock_gettime(&tv);

	printf("%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

static
void usage(int rc) {
	printf("usage: tikitank [-hv][-p PORT]\n");
	exit(rc);	
}

int main(int argc, char** argv) {
	int rc;
	const char* port = "80";
	struct pal* p;
	struct engine* eng;

	while ((rc = getopt(argc, argv, "hvp:")) != -1) {
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
		default:
			usage(2);
			break;
		}
	}

	// enc_thresh, enc_delay, ema_pow
	p = pal_init(1250, 1, 0);
	if (!p)
		return -1;

	eng = engine_init(p);
	rc = web_init(eng, port);
	if (rc) {
		LOG(("Web server failed to start: (%d) %s\n", rc, strerror(rc)));
		return rc;
	}

	settings_load();

	LOG(("Running engine\n"));

	web_run();
	engine_run();

	web_destroy();
	engine_destroy();
	pal_destroy();

	LOG(("Exited gracefully\n"));

	return 0;
}
