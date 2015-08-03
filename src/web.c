#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "mongoose.h"
#include "frozen.h"
#include "common.h"

#define NUM_PANELS  10
#define MAX_EFFECTS 10
#define ARG_SIZE    100

struct web_settings {
	long                dmx_brightness;
	long                manual_tick;
	long                idle_interval;
};

struct web_effect {
	const char*         id;
	int                 screen_saver;
};

union web_color {
	long                value;
	long                values[NUM_PANELS];
};

struct web_active {
	const char*         id;
	char                argument[ARG_SIZE];
	const char*         arg_desc;
	union web_color     color;
	int                 screen_saver;
	int                 sensor_driven;
};

struct web_channel {
	struct web_effect   all[MAX_EFFECTS];
	struct web_active   active;
};

struct web_effects {
	struct web_channel  treads;
	struct web_channel  panels;
	struct web_channel  barrel;
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

const char* JSON_DOC =
"{\n"
	"s : S"             // settings: JSON_SETTINGS
	"s : S"             // effects: JSON_EFFECTS
"}\n";

const char* JSON_SETTINGS =
"{\n"
	"\ts : i,\n"        // dmxBrightness: 0
	"\ts : i,\n"        // manualTick: 0
	"\ts : i\n"         // idleInterval: 0
"}\n";

const char* JSON_EFFECTS =
"{\n"
	"\ts : S,\n"        // treads: JSON_CHANNEL
	"\ts : S,\n"        // panels: JSON_CHANNEL
	"\ts : S\n"         // barrel: JSON_CHANNEL 
"}\n";

const char* JSON_CHANNEL =
"{\n"
	"\ts : [\n\tS\n],\n"// all: JSON_EFFECT
	"\ts : {\n"         // active:
		"\t\ts : s,\n"  // id: ""
		"\t\ts : S,\n"  // color: 0 | [0]
		"\t\ts : s,\n"  // argument: ""
		"\t\ts : s,\n"  // argumentDescription: ""
		"\t\ts : S,\n"  // isScreenSaver true|false
		"\t\ts : S\n"   // isSensorDriven: true|false
	"\t}\n"
"}";

const char* JSON_EFFECT =
"{\n"
	"\ts : s,\n"        // id: ""
	"\ts : S\n"         // isScreenSaver: true|false
"}\n";

const char* JSON_PANELS_COLOR = "[i, i, i, i, i, i, i, i, i, i]";

#define DEFAULT_DMX_BRIGHTNESS  100
#define DEFAULT_MANUAL_TICK       0
#define DEFAULT_IDLE_INTERVAL   120

#define SETTINGS_FILE      "settings.json"

#define SETTINGS           "settings"
#define DMX_BRIGHTNESS     "dmxBrightness"
#define MANUAL_TICK        "manualTick"
#define IDLE_INTERVAL      "idleInterval"
#define ARGUMENT           "argument"
#define ARGUMENT_DESC      "argumentDescription"
#define COLOR              "color"
#define IS_SSAVER          "isScreenSaver"
#define IS_SDRIVEN         "isSensorDriven"
#define ID                 "id"
#define EFFECTS            "effects"
#define TREADS             "treads"
#define PANELS             "panels"
#define BARREL             "barrel"
#define ALL                "all"
#define ACTIVE             "active"

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
void load_long(struct json_token* tokens, const char* path, long* into) {
	const struct json_token* token;

	token = find_json_token(tokens, path);
	if (token) {
		*into = strntol(token->ptr, token->len, 10);
	}
}

static
int settings_load() {
	FILE* fp;
	size_t len;
	int ret;

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

	load_long(tokens, SETTINGS "." DMX_BRIGHTNESS, &web.settings.dmx_brightness);
	load_long(tokens, SETTINGS "." MANUAL_TICK,    &web.settings.manual_tick);
	load_long(tokens, SETTINGS "." IDLE_INTERVAL,  &web.settings.idle_interval);

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
int channel_json(char* buf, size_t len, struct web_channel* channel) {
	int i;
	int ret;
	int first = 1;
	char color_buf[1024];
	char all_buf[10 * 1024];
	char* ptr = all_buf;

	*ptr = 0;

	for (i = 0; i < MAX_EFFECTS; i++) {
		struct web_effect* effect = &channel->all[i];
		if (effect->id) {
			char effect_buf[100];

			json_emit(effect_buf, sizeof(effect_buf), JSON_EFFECT,
				ID, effect->id,
				IS_SSAVER, effect->screen_saver ? "true" : "false"
			);

			if (first) {
				first = 0;
				ret = sprintf(ptr, "%s", effect_buf);
			} else {
				ret = sprintf(ptr, ", %s", effect_buf);
			}

			ptr += ret;
		}
	}

	if (channel == &web.effects.panels) {
		long* values = channel->active.color.values;
		json_emit(color_buf, sizeof(color_buf), JSON_PANELS_COLOR,
			values[0],
			values[1],
			values[2],
			values[3],
			values[4],
			values[5],
			values[6],
			values[7],
			values[8],
			values[9]
		);
	} else {
		json_emit_long(color_buf, sizeof(color_buf), channel->active.color.value);
	}

	return json_emit(buf, len, JSON_CHANNEL,
		ALL, all_buf,
		ACTIVE, 
			ID,            channel->active.id,
			COLOR,         color_buf,
			ARGUMENT,      channel->active.argument,
			ARGUMENT_DESC, channel->active.arg_desc,
			IS_SSAVER,     channel->active.screen_saver ? "true" : "false",
			IS_SDRIVEN,    channel->active.sensor_driven ? "true" : "false"
	);
}

static
int effects_json(char* buf, size_t len) {
	char treads_buf[10 * 1024];
	char panels_buf[10 * 1024];
	char barrel_buf[10 * 1024];

	channel_json(treads_buf, sizeof(treads_buf), &web.effects.treads);
	channel_json(panels_buf, sizeof(panels_buf), &web.effects.panels);
	channel_json(barrel_buf, sizeof(barrel_buf), &web.effects.barrel);

	return json_emit(buf, len, JSON_EFFECTS,
		TREADS, treads_buf,
		PANELS, panels_buf,
		BARREL, barrel_buf
	);
}

static
int settings_save() {
	FILE* fp;
	size_t len;
	char settings_buf[128];
	char effects_buf[10 * 1024];

	len = settings_json(settings_buf, sizeof(settings_buf));
	LOG(("settings_json: %d bytes\n", len));
	len = effects_json(effects_buf, sizeof(effects_buf));
	LOG(("effects_json: %d bytes\n", len));

	len = json_emit(work_buf, sizeof(work_buf), JSON_DOC,
		SETTINGS, settings_buf,
		EFFECTS, effects_buf
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
		web.settings.dmx_brightness = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, MANUAL_TICK, buf, sizeof(buf));
	if (len > 0) {
		web.settings.manual_tick = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, IDLE_INTERVAL, buf, sizeof(buf));
	if (len > 0) {
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
void effects_get(struct mg_connection* conn) {
	int len;

	settings_load();

	len = effects_json(work_buf, sizeof(work_buf));

	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_send_status(conn, 200);
	mg_send_data(conn, work_buf, len);
}

static
void effects_post(struct mg_connection* conn) {
	int rc;
	int len;

	settings_load();

	// TODO

	rc = settings_save();
	if (rc) {
		mg_printf_data(conn, "Could not save " SETTINGS_FILE);
		mg_send_status(conn, 500);
		return;
	}

	len = effects_json(work_buf, sizeof(work_buf));

	mg_send_header(conn, CONTENT_TYPE, CONTENT_TYPE_JSON);
	mg_send_status(conn, 200);
	mg_send_data(conn, work_buf, len);
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
		effects_get(conn);
	} else if (!strcmp(conn->request_method, "POST")) {
		effects_post(conn);
	}
}

int event_handler(struct mg_connection* conn, enum mg_event ev) {
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

	memset(&web.effects, sizeof(web.effects), 0);

	web.effects.treads.all[0].id = "rainbow";
	web.effects.treads.all[1].id = "foo";
	web.effects.treads.all[2].id = "bar";
	web.effects.treads.active.id = "rainbow";
	// web.effects.treads.active.argument = "";
	web.effects.treads.active.arg_desc = "";
	
	web.effects.panels.all[0].id = "foo";
	web.effects.panels.all[1].id = "bar";
	web.effects.panels.all[2].id = "rainbow";
	web.effects.panels.active.id = "rainbow";
	// web.effects.panels.active.argument = "";
	web.effects.panels.active.arg_desc = "";
	
	web.effects.barrel.all[0].id = "rainbow";
	web.effects.barrel.active.id = "rainbow";
	// web.effects.barrel.active.argument = "";
	web.effects.barrel.active.arg_desc = "";

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
