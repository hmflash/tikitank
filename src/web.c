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

struct web_effects_item {
	const char*  id;
	int          isScreenSaver;
};

union web_effect_argument {
	const char*  argument;
	long*        panels_colors;
};

struct web_effect_active {
	const char*                id;
	union web_effect_argument  argument;
	const char*                argumentDescription;
	int                        isScreenSaver;
	int                        isSensorDriven;
};

struct web_effect {
	struct web_effects_item* effects;
	struct web_effect_active active;
};

struct web_effects {
	struct web_effect treads;
	struct web_effect panels;
	struct web_effect barrel;
};

struct web_context {
	pthread_t           thread;
	struct mg_server*   server;
	struct engine*      engine;
	volatile int        exit;
	struct web_settings settings;
	struct web_effects  effects;
};

static 
struct web_context web;

static
const char* API_SETTINGS = "/api/settings";

static
const char* API_EFFECTS  = "/api/effects";

static
char work_buf[64 * 1024];

static
struct json_token tokens[100];

static
int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

const char* JSON_SETTINGS =
"{\n"                 
	"\ts : i,\n"        // dmxBrightness 
	"\ts : i,\n"        // manualTick 
	"\ts : i\n"         // idleInterval 
"}\n";

const char* JSON_LIST =
"[\n"                 
	"\tS\n"             // JSON_EFFECTS_ITEM
"]\n";

const char* JSON_EFFECTS_ITEM =
"{\n"                 
	"\ts : i,\n"        // id
	"\ts : i\n"         // isScreenSaver
"}\n";

const char* JSON_KIND =
"{\n"                 
	"\ts : S,\n"        // effects
	"\ts : S\n"         // active
"}\n";

const char* JSON_TREADS =
"{\n"                 
	"\ts : s,\n"        // id
	"\ts : s,\n"        // argument
	"\ts : s,\n"        // argumentDescription
	"\ts : S,\n"        // isScreenSaver
	"\ts : S\n"         // isSensorDriven
"}\n";

const char* JSON_BARREL =
"{\n"                 
	"\ts : s,\n"        // id
	"\ts : s,\n"        // argument
	"\ts : s,\n"        // argumentDescription
	"\ts : S\n"         // isScreenSaver
"}\n";

const char* JSON_PANELS =
"{\n"                 
	"\ts : s,\n"        // id
	"\ts : S\n"         // argument
"}\n";

const char* JSON_DOC =
"{\n"                 
	"s : S"             // settings
	"s : S"             // effects
"}\n";

#define DEFAULT_DMX_BRIGHTNESS  100
#define DEFAULT_MANUAL_TICK       0
#define DEFAULT_IDLE_INTERVAL   120

#define SETTINGS_FILE  "settings.json"

#define SETTINGS       "settings"
#define DMX_BRIGHTNESS "dmxBrightness"
#define MANUAL_TICK    "manualTick"
#define IDLE_INTERVAL  "idleInterval"
#define ARGUMENT       "argument"
#define ARGUMENT_DESC  "argumentDescription"
#define IS_SSAVER      "isScreenSaver"
#define IS_SDRIVEN     "isSensorDriven"
#define ID             "id"
#define EFFECTS        "effects"

#define CONTENT_TYPE       "Content-Type"
#define CONTENT_TYPE_JSON  "application/json"

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
	char settings_buf[1024];

	LOG(("settings_save> dmx_brightness: %ld\n", web.settings.dmx_brightness));
	LOG(("settings_save> manual_tick: %ld\n", web.settings.manual_tick));
	LOG(("settings_save> idle_interval: %ld\n", web.settings.idle_interval));

	settings_json(settings_buf, sizeof(settings_buf));
	
	len = json_emit(work_buf, sizeof(work_buf), JSON_DOC,
		SETTINGS, settings_buf
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
