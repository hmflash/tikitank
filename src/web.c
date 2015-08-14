#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "mongoose.h"
#include "frozen.h"
#include "common.h"
#include "effects/effects.h"
#include "engine.h"

struct web_context {
	pthread_t           thread;
	struct mg_server*   server;
	struct engine*      engine;
	volatile int        exit;
};

struct settings settings;

static 
struct web_context web;

static
struct mg_connection* ws_conn;

static
const char* API_SETTINGS = "/api/settings";

static
const char* API_EFFECTS  = "/api/effects";

static
char work_buf[64 * 1024];

static
struct json_token tokens[1024];

static
int tokens_size = sizeof(tokens) / sizeof(tokens[0]);

const char* JSON_DOC =
"{\n"
	"\ts : S,\n"          // settings: JSON_SETTINGS
	"\ts : S"             // effects: JSON_EFFECTS
"}\n";

const char* JSON_SETTINGS =
"{\n"
	"\t\ts : i,\n"        // dmxBrightness: 0
	"\t\ts : i,\n"        // manualTick: 0
	"\t\ts : i\n"         // idleInterval: 0
"\t}";

const char* JSON_EFFECTS =
"{\n"
	"\t\ts : S,\n"        // treads: JSON_CHANNEL
	"\t\ts : S,\n"        // panels: JSON_CHANNEL
	"\t\ts : S\n"         // barrel: JSON_CHANNEL 
"\t}\n";

const char* JSON_CHANNEL =
"{\n"
	"\t\t\ts : i,\n"      // active: 0
	"\t\t\ts : [\n"
		"S\n"             // all: JSON_EFFECT
	"\t\t\t]\n"
"\t\t}";

const char* JSON_EFFECT =
"\t\t\t\t{\n"
	"\t\t\t\t\ts : s,\n"  // name: ""
	"\t\t\t\t\ts : i,\n"  // argument: 0
	"\t\t\t\t\ts : s,\n"  // argumentDescription: ""
	"\t\t\t\t\ts : S,\n"  // color: 0|[0]
	"\t\t\t\t\ts : i,\n"  // isScreenSaver 0|1
	"\t\t\t\t\ts : i\n"   // isSensorDriven: 0|1
"\t\t\t\t}";

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
#define NAME               "name"
#define EFFECTS            "effects"
#define TREADS             "treads"
#define PANELS             "panels"
#define BARREL             "barrel"
#define ALL                "all"
#define ACTIVE             "active"
#define KIND               "kind"

#define CONTENT_TYPE       "Content-Type"
#define CONTENT_TYPE_JSON  "application/json"

static 
long strntol(const char* buf, size_t size, int base) {
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
void channel_load(struct json_token* tokens, const char* kind, struct channel* channel) {
	char path[256];
	long active;
	const struct json_token* token;

	// LOG(("channel_load> %s\n", kind));
	
	snprintf(path, sizeof(path), EFFECTS ".%s." ALL, kind);
	token = find_json_token(tokens, path);
	if (token) {
		int i;

		for (i = 0; i < channel->num_effects && i < token->num_desc; i++) {
			snprintf(path, sizeof(path), EFFECTS ".%s." ALL "[%d]" ARGUMENT, kind, i);
			load_long(tokens, path, &channel->effects[i]->argument);

			snprintf(path, sizeof(path), EFFECTS ".%s." ALL "[%d]" IS_SSAVER, kind, i);
			load_long(tokens, path, &channel->effects[i]->screen_saver);

			snprintf(path, sizeof(path), EFFECTS ".%s." ALL "[%d]" IS_SDRIVEN, kind, i);
			load_long(tokens, path, &channel->effects[i]->sensor_driven);
			
			if (channel == &channel_panels) {
				int j;

				for (j = 0; j < NUM_PANELS/3; j++) {
					snprintf(path, sizeof(path), EFFECTS ".%s." ALL "[%d]" COLOR "[%d]", kind, i, j);
					load_long(tokens, path, &channel->effects[i]->color_arg.colors[j].value);
				}
			} else {
				snprintf(path, sizeof(path), EFFECTS ".%s." ALL "[%d]" COLOR, kind, i);
				load_long(tokens, path, &channel->effects[i]->color_arg.color.value);
			}
		}
	}

	snprintf(path, sizeof(path), EFFECTS ".%s." ACTIVE, kind);
	load_long(tokens, path, &active);
	if (active < channel->num_effects && channel->effects[active]->name) {
		channel->active = active;
	}
}

int settings_load() {
	FILE* fp;
	size_t len;
	int ret;

	settings.dmx_brightness = DEFAULT_DMX_BRIGHTNESS;
	settings.manual_tick    = DEFAULT_MANUAL_TICK;
	settings.idle_interval  = DEFAULT_IDLE_INTERVAL;

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

	load_long(tokens, SETTINGS "." DMX_BRIGHTNESS, &settings.dmx_brightness);
	load_long(tokens, SETTINGS "." MANUAL_TICK,    &settings.manual_tick);
	load_long(tokens, SETTINGS "." IDLE_INTERVAL,  &settings.idle_interval);

	channel_load(tokens, TREADS, &channel_treads);
	channel_load(tokens, PANELS, &channel_panels);
	channel_load(tokens, BARREL, &channel_barrel);

	return 0;
}

static
int settings_json(char* buf, size_t len) {
	return json_emit(buf, len, JSON_SETTINGS, 
		DMX_BRIGHTNESS, settings.dmx_brightness, 
		MANUAL_TICK,    settings.manual_tick, 
		IDLE_INTERVAL,  settings.idle_interval
	);
}

static
int color_json(char* buf, size_t len, struct channel* channel, union color_arg* arg) {
	if (channel == &channel_panels) {
		union color* values = arg->colors;
		return json_emit(buf, len, JSON_PANELS_COLOR,
			values[0].value,
			values[1].value,
			values[2].value,
			values[3].value,
			values[4].value,
			values[5].value,
			values[6].value,
			values[7].value,
			values[8].value,
			values[9].value
		);
	} 
	return json_emit_long(buf, len, arg->color.value);
}

static 
int channel_json(char* buf, size_t len, struct channel* channel) {
	int i;
	int ret;
	int first = 1;
	char all_buf[MAX_EFFECTS * 1024];
	char* ptr = all_buf;

	*ptr = 0;

	// LOG(("channel_json> %p\n", channel));

	for (i = 0; i < channel->num_effects; i++) {
		struct effect* effect = channel->effects[i];
		char color_buf[1024];
		char effect_buf[1024];
	
		// LOG(("channel_json> effect: %s\n", effect->name));

		color_json(color_buf, sizeof(color_buf), channel, &effect->color_arg);

		json_emit(effect_buf, sizeof(effect_buf), JSON_EFFECT,
			NAME,          effect->name,
			ARGUMENT,      effect->argument,
			ARGUMENT_DESC, effect->arg_desc ? effect->arg_desc : "",
			COLOR,         color_buf,
			IS_SSAVER,     effect->screen_saver,
			IS_SDRIVEN,    effect->sensor_driven
		);

		if (first) {
			first = 0;
			ret = sprintf(ptr, "%s", effect_buf);
		} else {
			ret = sprintf(ptr, ",\n%s", effect_buf);
		}

		ptr += ret;
	}

	return json_emit(buf, len, JSON_CHANNEL,
		ACTIVE, channel->active,
		ALL,    all_buf
	);
}

static
int effects_json(char* buf, size_t len) {
	char treads_buf[MAX_EFFECTS * 1024];
	char panels_buf[MAX_EFFECTS * 1024];
	char barrel_buf[MAX_EFFECTS * 1024];

	channel_json(treads_buf, sizeof(treads_buf), &channel_treads);
	channel_json(panels_buf, sizeof(panels_buf), &channel_panels);
	channel_json(barrel_buf, sizeof(barrel_buf), &channel_barrel);

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
	char effects_buf[MAX_EFFECTS * 1024];

	len = settings_json(settings_buf, sizeof(settings_buf));
	LOG(("settings_json: %zu bytes\n", len));
	len = effects_json(effects_buf, sizeof(effects_buf));
	LOG(("effects_json: %zu bytes\n", len));

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
		settings.dmx_brightness = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, MANUAL_TICK, buf, sizeof(buf));
	if (len > 0) {
		settings.manual_tick = strtol(buf, NULL, 10);
	}

	len = mg_get_var(conn, IDLE_INTERVAL, buf, sizeof(buf));
	if (len > 0) {
		settings.idle_interval = strtol(buf, NULL, 10);
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
void effects_post_reply(struct mg_connection* conn) {
	int rc;
	int len;

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
void effects_post(struct mg_connection* conn) {
	int len1;
	int len2;
	char kind[256];
	char arg1[256];
	char arg2[256];
	struct channel* channel;

	settings_load();

	len1 = mg_get_var(conn, KIND, kind, sizeof(kind));
	if (len1 < 0) {
		mg_printf_data(conn, "Missing field: " KIND);
		mg_send_status(conn, 400);
		return;
	}

	if(!strncmp(kind, "treads", sizeof(kind))) {
		channel = &channel_treads;
	} else if(!strncmp(kind, "barrel", sizeof(kind))) {
		channel = &channel_barrel;
	} else if(!strncmp(kind, "panels", sizeof(kind))) {
		channel = &channel_panels;
	} else {
		mg_printf_data(conn, "Invalid kind");
		mg_send_status(conn, 400);
		return;
	}
	
	len1 = mg_get_var(conn, ACTIVE, arg1, sizeof(arg1));
	if (len1 > 0) {
		long idx;

		LOG(("selectEffect: kind=%s active=%s\n", kind, arg1));

		idx = strntol(arg1, len1, 10);
		if (idx >= 0 && idx < channel->num_effects) {
			channel->active = idx;
			effects_post_reply(conn);
			return;
		}

		mg_printf_data(conn, "Invalid effect");
		mg_send_status(conn, 400);
		return;
	}

	len1 = mg_get_var(conn, IS_SSAVER, arg1, sizeof(arg1));
	if (len1 > 0) {
		LOG(("setEffectScreenSaver: %s\n", arg1));
		if (!strncmp(arg1, "true", sizeof(arg1))) {
			channel->effects[channel->active]->screen_saver = 1;
		} else {
			channel->effects[channel->active]->screen_saver = 0;
		}
		effects_post_reply(conn);
		return;
	}

	len1 = mg_get_var(conn, IS_SDRIVEN, arg1, sizeof(arg1));
	if (len1 > 0) {
		LOG(("setEffectSensorDrive: %s\n", arg1));
		if (!strncmp(arg1, "true", sizeof(arg1))) {
			channel->effects[channel->active]->sensor_driven = 1;
		} else {
			channel->effects[channel->active]->sensor_driven = 0;
		}
		effects_post_reply(conn);
		return;
	}

	len1 = mg_get_var(conn, COLOR, arg1, sizeof(arg1));
	len2 = mg_get_var(conn, ARGUMENT, arg2, sizeof(arg2));

	if (len1 > 0 && len2 > 0) {
		int idx;
		LOG(("setEffectParameters: %s, %s\n", arg1, arg2));
		idx = strntol(arg2, len2, 10);
		if (idx < NUM_PANELS/3) {
			channel->effects[channel->active]->color_arg.colors[idx].value = strntol(arg1+1, len1-1, 16);
		}
		effects_post_reply(conn);
		return;
	} else if (len1 > 0) {
		LOG(("setEffectColor: %s\n", arg1));
		channel->effects[channel->active]->color_arg.color.value = strntol(arg1+1, len1-1, 16);
		effects_post_reply(conn);
		return;
	} else if (len2 > 0) {
		LOG(("setEffectArgument: %s\n", arg2));
		channel->effects[channel->active]->argument = strntol(arg2, len2, 10);
		effects_post_reply(conn);
		return;
	}

	mg_printf_data(conn, "Invalid data");
	mg_send_status(conn, 400);
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

static
int on_request(struct mg_connection* conn) {
	int ret = MG_FALSE;

	// LOG(("MG_REQUEST:      %p %d %d\n", conn, conn->is_websocket, conn->wsbits));

	engine_lock();

	if (!strncmp(conn->uri, API_SETTINGS, strlen(API_SETTINGS))) {
		on_settings(conn);
		ret = MG_TRUE;
	} else if (!strncmp(conn->uri, API_EFFECTS, strlen(API_EFFECTS))) {
		on_effects(conn);
		ret = MG_TRUE;
	}

	engine_unlock();

	return ret;
}

int event_handler(struct mg_connection* conn, enum mg_event ev) {
	switch (ev) {
	case MG_AUTH: 
		return MG_TRUE;
	case MG_REQUEST:
		return on_request(conn);
	case MG_WS_HANDSHAKE:
		// LOG(("MG_WS_HANDSHAKE: %p %d %d\n", conn, conn->is_websocket, conn->wsbits));
		return MG_FALSE;
	case MG_WS_CONNECT:
		LOG(("MG_WS_CONNECT:   %p %d %d\n", conn, conn->is_websocket, conn->wsbits));
		ws_conn = conn;
		return MG_TRUE;
	case MG_CLOSE:
		if (conn == ws_conn) {
			LOG(("MG_CLOSE:        %p %d %d\n", conn, conn->is_websocket, conn->wsbits));
			ws_conn = NULL;
		}
		return MG_TRUE;
	default: 
		return MG_FALSE;
	}
}

static 
void* web_thread(void* arg) {
	while (!web.exit) {
		mg_poll_server(web.server, 20);
	}

	mg_destroy_server(&web.server);
	return NULL;
}

int web_init(struct engine* eng, const char* port) {
	memset(&web, 0, sizeof(web));
	web.engine = eng;
	web.server = mg_create_server(NULL, event_handler);

	mg_set_option(web.server, "document_root", "static");
	mg_set_option(web.server, "listening_port", port);
	
	errno = 0;

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

void web_treads_render(const char* buf, size_t len) {
	// LOG(("render: %zu bytes\n", len));
	if (ws_conn) {
		mg_websocket_write(ws_conn, WEBSOCKET_OPCODE_BINARY, buf, len);
	}
}
