#include <stdlib.h>
#include <string.h>
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

static inline
uint8_t scale_color(uint8_t color, uint8_t scale) {
	return ((int)color * (int)(scale)) >> 9;
}

static
void hover(struct render_args* args) {
	union color color = args->effect->color_arg.color;
	int num_pixels = args->framelen/3;
	int upper = args->framelen + 85;
	int lower = args->framelen + 85;
	int keyframe = num_pixels/2;

	memset(args->framebuf, 0x80, args->framelen);

	int k;
	for (k = 0; k < keyframe; k++) {
		int m = (k + args->shift_quotient) % keyframe;
		int i = (lower + m) % num_pixels;
		int j = (upper - m) % num_pixels;
		int cycle = k % 15;

		if (cycle == 9) {         // leading edge
			args->framebuf[i*3+0] = 0x80 | scale_color(color.rgb.green, 0xff - args->shift_remainder);
			args->framebuf[i*3+1] = 0x80 | scale_color(color.rgb.red,   0xff - args->shift_remainder);
			args->framebuf[i*3+2] = 0x80 | scale_color(color.rgb.blue,  0xff - args->shift_remainder);
	
			args->framebuf[j*3+0] = 0x80 | scale_color(color.rgb.green, 0xff - args->shift_remainder);
			args->framebuf[j*3+1] = 0x80 | scale_color(color.rgb.red,   0xff - args->shift_remainder);
			args->framebuf[j*3+2] = 0x80 | scale_color(color.rgb.blue,  0xff - args->shift_remainder);
		} else if (cycle == 14) { // trailing edge
			args->framebuf[i*3+0] = 0x80 | scale_color(color.rgb.green, args->shift_remainder);
			args->framebuf[i*3+1] = 0x80 | scale_color(color.rgb.red,   args->shift_remainder);
			args->framebuf[i*3+2] = 0x80 | scale_color(color.rgb.blue,  args->shift_remainder);
	
			args->framebuf[j*3+0] = 0x80 | scale_color(color.rgb.green, args->shift_remainder);
			args->framebuf[j*3+1] = 0x80 | scale_color(color.rgb.red,   args->shift_remainder);
			args->framebuf[j*3+2] = 0x80 | scale_color(color.rgb.blue,  args->shift_remainder);
		} else if (cycle >= 10) { // middle
			args->framebuf[i*3+0] = 0x80 | color.rgb.green;
			args->framebuf[i*3+1] = 0x80 | color.rgb.red;
			args->framebuf[i*3+2] = 0x80 | color.rgb.blue;

			args->framebuf[j*3+0] = 0x80 | color.rgb.green;
			args->framebuf[j*3+1] = 0x80 | color.rgb.red;
			args->framebuf[j*3+2] = 0x80 | color.rgb.blue;
		}
	}
}

struct effect effect_treads_hover = {
	.name          = "hover",
	.render        = hover,
	.sensor_driven = 1,
};

static
void rainbow_hover(struct render_args* args) {
	int num_pixels = args->framelen/3;
	int upper = args->framelen + 85;
	int lower = args->framelen + 85;
	int keyframe = num_pixels/2;
	int f = args->framenum;

	memset(args->framebuf, 0x80, args->framelen);

	int k;
	for (k = 0; k < keyframe; k++) {
		int m = (k + args->shift_quotient) % keyframe;
		int i = (lower + m) % num_pixels;
		int j = (upper - m) % num_pixels;
		int cycle = k % 15;
		int c = k * 3;

		if (cycle == 9) {         // leading edge
			args->framebuf[i*3+0] = 0x80 | scale_color(RAINBOW_G(c), 0xff - args->shift_remainder);
			args->framebuf[i*3+1] = 0x80 | scale_color(RAINBOW_R(c), 0xff - args->shift_remainder);
			args->framebuf[i*3+2] = 0x80 | scale_color(RAINBOW_B(c), 0xff - args->shift_remainder);
	
			args->framebuf[j*3+0] = 0x80 | scale_color(RAINBOW_G(c), 0xff - args->shift_remainder);
			args->framebuf[j*3+1] = 0x80 | scale_color(RAINBOW_R(c), 0xff - args->shift_remainder);
			args->framebuf[j*3+2] = 0x80 | scale_color(RAINBOW_B(c), 0xff - args->shift_remainder);
		} else if (cycle == 14) { // trailing edge
			args->framebuf[i*3+0] = 0x80 | scale_color(RAINBOW_G(c), args->shift_remainder);
			args->framebuf[i*3+1] = 0x80 | scale_color(RAINBOW_R(c), args->shift_remainder);
			args->framebuf[i*3+2] = 0x80 | scale_color(RAINBOW_B(c), args->shift_remainder);
	
			args->framebuf[j*3+0] = 0x80 | scale_color(RAINBOW_G(c), args->shift_remainder);
			args->framebuf[j*3+1] = 0x80 | scale_color(RAINBOW_R(c), args->shift_remainder);
			args->framebuf[j*3+2] = 0x80 | scale_color(RAINBOW_B(c), args->shift_remainder);
		} else if (cycle >= 10) { // middle
			args->framebuf[i*3+0] = 0x80 | RAINBOW_G(c) >> 1;
			args->framebuf[i*3+1] = 0x80 | RAINBOW_R(c) >> 1;
			args->framebuf[i*3+2] = 0x80 | RAINBOW_B(c) >> 1;

			args->framebuf[j*3+0] = 0x80 | RAINBOW_G(c) >> 1;
			args->framebuf[j*3+1] = 0x80 | RAINBOW_R(c) >> 1;
			args->framebuf[j*3+2] = 0x80 | RAINBOW_B(c) >> 1;
		}
	}
}

struct effect effect_treads_rainbow_hover = {
	.name          = "rainbow hover",
	.render        = rainbow_hover,
	.sensor_driven = 1,
};
