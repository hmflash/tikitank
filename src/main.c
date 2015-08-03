#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "engine.h"
#include "web.h"

static 
struct pal pal;

void debug_log(const char const* fmt, ...) {
	va_list arg;
	struct timespec tv;

	clock_gettime(CLOCK_MONOTONIC, &tv);

	printf("%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

int main(int argc, char** argv) {
	struct engine* eng;
	int rc;

	if (pal_init(&pal))
		return -1;

	eng = engine_init(&pal);
	rc = web_init(eng);
	if (rc) {
		LOG(("Web server failed to start: (%d) %s\n", rc, strerror(rc)));
		return rc;
	}

	LOG(("Running engine\n"));

	web_run();
	engine_run();

	web_destroy();
	engine_destroy();
	pal_destroy(&pal);

	LOG(("Exited gracefully\n"));

	return 0;
}
