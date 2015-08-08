#pragma once

#define NUM_PANELS      10

union color {
	long                value;
	long                values[NUM_PANELS];
};

struct effect {
	const char*         name;
	long                argument;
	const char*         arg_desc;
	union color         color;
	long                screen_saver;
	long                sensor_driven;

	void (*render_treads)(struct effect* effect, int framenum, char* framebuf, size_t framelen);
	void (*render_barrel)(struct effect* effect, int framenum, char* framebuf, size_t framelen);
	void (*render_panels)(struct effect* effect, int framenum, char* framebuf, size_t framelen);
};

extern struct effect* effects_table[];
