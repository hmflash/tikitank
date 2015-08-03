#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "mongoose.h"
#include "frozen.h"
#include "common.h"

struct web_settings {
	long dmx_brightness;
	long manual_tick;
	long idle_interval;
};

struct web_context {
	pthread_t           thread;
	struct mg_server*   server;
	struct engine*      engine;
	volatile int        exit;
	struct web_settings settings;
};

struct web_effects {
};

static 
struct web_context web;

static
const char* API_SETTINGS = "/api/settings";

static
const char* API_EFFECTS = "/api/effects";

static
char work_buf[64 * 1024];

// static
// struct char work_vars[1024][10];

static
struct json_token tokens[100];

static
int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

#define JSON_SETTINGS \
"{\n"                 \
	"\ts : i,\n"      \
	"\ts : i,\n"      \
	"\ts : i\n"       \
"}\n"

#define JSON_DOC           \
"{\n"                      \
	"s : " JSON_SETTINGS   \
"}\n"

#define DEFAULT_DMX_BRIGHTNESS  100
#define DEFAULT_MANUAL_TICK       0
#define DEFAULT_IDLE_INTERVAL   120

#define SETTINGS_FILE  "settings.json"
#define SETTINGS       "settings"
#define DMX_BRIGHTNESS "dmxBrightness"
#define MANUAL_TICK    "manualTick"
#define IDLE_INTERVAL  "idleInterval"

#define CONTENT_TYPE       "Content-Type"
#define CONTENT_TYPE_JSON  "application/json"

/* settings
{
	dmxBrightness: 0,
	manualTick: 0,
	idleInterval: 0
}
*/

/* effect
GET
{
	treads: {
		kind: "treads",
		effects: [
			{
				id: "",
				name: "",
				isScreenSaver: true
			}
		],
		active: {
			id: "",
			name: "",
			argument: "",
			argumentDescription: "",
			isScreenSaver: true,
			isSensorDriven: true
		}
	},
	panels: {
		kind: "panels",
		effects: [],
		active: {
			id: "",
			name: "",
			argument: "",
			argumentDescription: "",
			isScreenSaver: true,
			isSensorDriven: true
		}
	},
	barrel: {
		kind: "barrel",
		effects: [],
		active: {
			id: "",
			name: "",
			argument: "",
			argumentDescription: "",
			isScreenSaver: true,
			isSensorDriven: true
		}
	}
}

POST
{
	kind: "",
	id: "",
	name: "",
	argument: "",
	argumentDescription: "",
	isScreenSaver: true,
	isSensorDriven: true
}
*/

static 
long strntol(const char* buf, size_t size, int base)
{
	static char str[100];
	if (size >= (sizeof(str) - 1)) {
		return 0;
	}

	memcpy(str, buf, size);
	str[size] = 0;

	return strtol(str, NULL, base);
}

static
int settings_load() {
	FILE* fp;
	size_t len;
	int ret;
	const struct json_token* token;

	web.settings.dmx_brightness = DEFAULT_DMX_BRIGHTNESS;
	web.settings.manual_tick    = DEFAULT_MANUAL_TICK;
	web.settings.idle_interval  = DEFAULT_IDLE_INTERVAL;

	fp = fopen(SETTINGS_FILE, "r");
	if (!fp) {
		LOG(("settings_load> fopen() failed: (%d) %s\n", errno, strerror(errno)));
		return errno;
	}

	len = fread(work_buf, 1, sizeof(work_buf), fp);
	work_buf[len] = 0;

	fclose(fp);
	
	LOG(("settings_load> %s\n", work_buf));
	
	ret = parse_json(work_buf, len, tokens, tokens_size);
	if (ret < 0) {
		LOG(("settings_load> parse_json() failed: %d\n", ret));
		return ret;
	}

	token = find_json_token(tokens, SETTINGS "." DMX_BRIGHTNESS);
	if (token) {
		web.settings.dmx_brightness = strntol(token->ptr, token->len, 10);
	}

	token = find_json_token(tokens, SETTINGS "." MANUAL_TICK);
	if (token) {
		web.settings.manual_tick = strntol(token->ptr, token->len, 10);
	}

	token = find_json_token(tokens, SETTINGS "." IDLE_INTERVAL);
	if (token) {
		web.settings.idle_interval = strntol(token->ptr, token->len, 10);
	}

	return 0;
}

static
int settings_json(char* buf, size_t len) {
	return json_emit(buf, len, JSON_SETTINGS, 
		DMX_BRIGHTNESS, web.settings.dmx_brightness, 
		MANUAL_TICK,    web.settings.manual_tick, 
		IDLE_INTERVAL,  web.settings.idle_interval
	);
}

static
int settings_save() {
	FILE* fp;
	size_t len;

	LOG(("settings_save> dmx_brightness: %ld\n", web.settings.dmx_brightness));
	LOG(("settings_save> manual_tick: %ld\n", web.settings.manual_tick));
	LOG(("settings_save> idle_interval: %ld\n", web.settings.idle_interval));
	
	len = json_emit(work_buf, sizeof(work_buf), JSON_DOC,
		SETTINGS,
			DMX_BRIGHTNESS, web.settings.dmx_brightness, 
			MANUAL_TICK,    web.settings.manual_tick, 
			IDLE_INTERVAL,  web.settings.idle_interval
	);

	fp = fopen(SETTINGS_FILE, "w");
	if (!fp) {
		LOG(("settings_save> fopen() failed: (%d) %s\n", errno, strerror(errno)));
		return errno;
	}

	fwrite(work_buf, len, 1, fp);

	fclose(fp);
	return 0;
}

static
void settings_get(struct mg_connection* conn) {
	int len;

	settings_load();

	len = settings_json(work_buf, sizeof(work_buf));

	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_send_status(conn, 200);
	mg_send_data(conn, work_buf, len);
}

static
void settings_post(struct mg_connection* conn) {
	int rc;
	int len;
	char buf[256];

	settings_load();

	len = mg_get_var(conn, DMX_BRIGHTNESS, buf, sizeof(buf));
	if (len > 0) {
		LOG(("settings_post> dmx_brightness: %d\n", len));
		web.settings.dmx_brightness = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, MANUAL_TICK, buf, sizeof(buf));
	if (len > 0) {
		LOG(("settings_post> manual_tick: %d\n", len));
		web.settings.manual_tick = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, IDLE_INTERVAL, buf, sizeof(buf));
	if (len > 0) {
		LOG(("settings_post> idle_interval: %d\n", len));
		web.settings.idle_interval = strtol(buf, NULL, 10);
	}

	rc = settings_save();
	if (rc) {
		mg_printf_data(conn, "Could not save " SETTINGS_FILE);
		mg_send_status(conn, 500);
		return;
	}

	len = settings_json(work_buf, sizeof(work_buf));

	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_send_status(conn, 200);
	mg_send_data(conn, work_buf, len);
}

static
void get_effects(struct mg_connection* conn) {
	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_printf_data(conn, "{}");
}

static
void post_effects(struct mg_connection* conn) {
	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_printf_data(conn, "{}");
}

static
void on_settings(struct mg_connection* conn) {
	LOG(("settings: %s\n", conn->uri));
	if (!strcmp(conn->request_method, "GET")) {
		settings_get(conn);
	} else if (!strcmp(conn->request_method, "POST")) {
		settings_post(conn);
	}
}

static
void on_effects(struct mg_connection* conn) {
	LOG(("effects: %s\n", conn->uri));
	if (!strcmp(conn->request_method, "GET")) {
		get_effects(conn);
	} else if (!strcmp(conn->request_method, "POST")) {
		post_effects(conn);
	}
}

int event_handler(struct mg_connection* conn, enum mg_event ev) {
	// LOG(("event_handler> %d\n", ev));

	switch (ev) {
	case MG_AUTH: 
		return MG_TRUE;
	case MG_REQUEST:
		if (!strncmp(conn->uri, API_SETTINGS, strlen(API_SETTINGS))) {
			on_settings(conn);
			return MG_TRUE;
		}
		if (!strncmp(conn->uri, API_EFFECTS, strlen(API_EFFECTS))) {
			on_effects(conn);
			return MG_TRUE;
		}
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
