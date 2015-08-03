#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "mongoose.h"
#include "common.h"

struct web_context {
	pthread_t          thread;
	struct mg_server*  server;
	struct engine*     engine;
	volatile int       exit;
};

static struct web_context web;

int event_handler(struct mg_connection* conn, enum mg_event ev) {
	// LOG(("event_handler> %d\n", ev));

	switch (ev) {
	case MG_AUTH: 
		return MG_TRUE;
	case MG_REQUEST:
	default: 
		return MG_FALSE;
	}
}

static 
void* web_thread(void* arg) {
	while (!web.exit) {
		mg_poll_server(web.server, 1000);
	}

	mg_destroy_server(&web.server);
	return NULL;
}

int web_init(struct engine* eng) {
	web.exit = 0;
	web.engine = eng;
	web.server = mg_create_server(NULL, event_handler);

	mg_set_option(web.server, "document_root", "static");
	mg_set_option(web.server, "listening_port", "9999");
	
	mg_poll_server(web.server, 0);

	return errno;
}

void web_run() {
	int rc;

	rc = pthread_create(&web.thread, NULL, web_thread, NULL);
	LOG(("Web server started: (%d) %s\n", rc, strerror(rc)));
}

void web_destroy() {
	int rc;

	web.exit = 1;

	rc = pthread_join(web.thread, NULL);
	LOG(("Web server stopped: (%d) %s\n", rc, strerror(rc)));
}
