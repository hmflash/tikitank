#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "effects.h"
#include "common.h"

// A: 79
// B: 35
// C: 350
// D: 330
// E: 319
// F: 299
// G: 136
// H: 90

#define KEYFRAME 227

static
void cylon_treads(struct render_args* args) {
	union color color = args->effect->color_arg.color;
	int num_pixels = args->framelen/3;
	int upper_start  = args->framelen + 330;
	int upper_finish = args->framelen + 79;
	int lower_start  = args->framelen + 319;
	int lower_finish = args->framelen + 90;

	memset(args->framebuf, 0x80, args->framelen);

	int state = args->framenum % (KEYFRAME * 2);
	if (state < KEYFRAME) {
		int k;
		for (k = -2; k < 3; k++) {
			int i = (lower_finish + state + k) % num_pixels;
			args->framebuf[i*3+0] = 0x80 | color.rgb.green;
			args->framebuf[i*3+1] = 0x80 | color.rgb.red;
			args->framebuf[i*3+2] = 0x80 | color.rgb.blue;
	
			int j = (upper_finish - state + k) % num_pixels;
			args->framebuf[j*3+0] = 0x80 | color.rgb.green;
			args->framebuf[j*3+1] = 0x80 | color.rgb.red;
			args->framebuf[j*3+2] = 0x80 | color.rgb.blue;
		}
	} else {
		int k;
		int s = state - KEYFRAME;
		for (k = -2; k < 3; k++) {
			int i = (lower_start - s + k) % num_pixels;
			args->framebuf[i*3+0] = 0x80 | color.rgb.green;
			args->framebuf[i*3+1] = 0x80 | color.rgb.red;
			args->framebuf[i*3+2] = 0x80 | color.rgb.blue;

			int j = (upper_start + s + k) % num_pixels;
			args->framebuf[j*3+0] = 0x80 | color.rgb.green;
			args->framebuf[j*3+1] = 0x80 | color.rgb.red;
			args->framebuf[j*3+2] = 0x80 | color.rgb.blue;
		}
	}
}

static
void cylon_barrel(struct render_args* args) {
	int i, j;
	union color color = args->effect->color_arg.color;
	int num_pixels = args->framelen/3;
	int cycle = args->framenum % num_pixels;
	double t = cos(2 * M_PI * (double)cycle / (double)num_pixels);
	int x = num_pixels/2 * t + num_pixels/2;
	int lx = x - 2;
	int rx = x + 2;

	for (i = 0, j = 0; i < num_pixels; i++, j += 3) {
		char* pixel = &args->framebuf[j];
		if (i >= lx && i <= rx) {
			pixel[0] = 0x80 | color.rgb.green;
			pixel[1] = 0x80 | color.rgb.red;
			pixel[2] = 0x80 | color.rgb.blue;
		} else {
			pixel[0] = 0x80;
			pixel[1] = 0x80;
			pixel[2] = 0x80;
		}
	}
}

static inline
int scale_color(int c, double scale) {
	if (scale == 1.0) {
		return c;
	}
	int nonzeroscale = (scale != 0) ? 1 : 0;
	return (c == 0) ? 0 : (c * scale) + nonzeroscale;
}

static inline
void set_color(char* buf, int len, int i, union color color, double scale) {
	if (0 <= i && i < len) {
		int p = i*3;
		buf[p+0] = 0x80 | scale_color(color.rgb.green, scale);
		buf[p+1] = 0x80 | scale_color(color.rgb.red,   scale);
		buf[p+2] = 0x80 | scale_color(color.rgb.blue,  scale);
	}
}

static
void kitt_barrel(struct render_args* args) {
	union color color = args->effect->color_arg.color;
	int num_pixels = args->framelen/3;
	int x = args->framenum % (num_pixels * 2);

	memset(args->framebuf, 0x80, args->framelen);

	if (x < num_pixels) {
		set_color(args->framebuf, num_pixels, x+1, color, 0.75);
		set_color(args->framebuf, num_pixels, x+0, color, 1.00);
		set_color(args->framebuf, num_pixels, x-1, color, 0.75);
		set_color(args->framebuf, num_pixels, x-2, color, 0.50);
		set_color(args->framebuf, num_pixels, x-3, color, 0.25);
		set_color(args->framebuf, num_pixels, x-4, color, 0.10);
	} else {
		x = num_pixels*2 - x;
		set_color(args->framebuf, num_pixels, x-1, color, 0.75);
		set_color(args->framebuf, num_pixels, x+0, color, 1.00);
		set_color(args->framebuf, num_pixels, x+1, color, 0.75);
		set_color(args->framebuf, num_pixels, x+2, color, 0.50);
		set_color(args->framebuf, num_pixels, x+3, color, 0.25);
		set_color(args->framebuf, num_pixels, x+4, color, 0.10);
	}
}

struct effect effect_treads_cylon = {
	.name          = "cylon",
	.render        = cylon_treads,
};

struct effect effect_barrel_cylon = {
	.name          = "cylon",
	.render        = cylon_barrel,
};

struct effect effect_barrel_kitt = {
	.name          = "kitt",
	.render        = kitt_barrel,
};

