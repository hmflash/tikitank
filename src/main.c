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

void debug_log(const char const* fmt, ...) {
	va_list arg;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	printf("%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

static
void usage(int rc) {
	printf("usage: tikitank [-h][-p PORT]\n");
	exit(rc);	
}

int main(int argc, char** argv) {
	int rc;
	const char* port = "80";
	struct pal* p;
	struct engine* eng;

	while ((rc = getopt(argc, argv, "hp:")) != -1) {
		switch (rc) {
		case 'p':
			port = optarg;
			break;
		case 'h':
			usage(0);
			break;
		default:
			usage(2);
			break;
		}
	}

	p = pal_init(425, 1); // enc_thresh & enc_delay
	if (!p)
		return -1;

	eng = engine_init(p);
	rc = web_init(eng, port);
	if (rc) {
		LOG(("Web server failed to start: (%d) %s\n", rc, strerror(rc)));
		return rc;
	}

	LOG(("Running engine\n"));

	web_run();
	engine_run();

	web_destroy();
	engine_destroy();
	pal_destroy();

	LOG(("Exited gracefully\n"));

	return 0;
}
