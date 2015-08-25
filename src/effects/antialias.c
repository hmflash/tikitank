#include <stdint.h>
#include <stddef.h>

#include "effects.h"

static inline
uint8_t scale_color(uint8_t color, uint8_t scale) {
	return ((int)color * (int)(scale)) >> 9;
}

static
void antialias_treads(struct render_args* args) {
	int i;
	int j;
	int cycle;
	int num_pixels = args->framelen/3;

	for (i = 0; i < num_pixels; i++) {
		char* pixel = &args->framebuf[i*3];
		union color color = args->effect->color_arg.color;
		get_cycle(args, i, &j, &cycle);

		if (cycle == 9) {         // leading edge
			pixel[0] = 0x80 | scale_color(color.rgb.green, args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.red,   args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue,  args->shift_remainder);
		} else if (cycle == 14) { // trailing edge
			pixel[0] = 0x80 | scale_color(color.rgb.green, 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.red,   0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue,  0xff - args->shift_remainder);
		} else if (cycle >= 10) { // middle
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

static
void rolling_rainbow_treads(struct render_args* args) {
	int i;
	int j;
	int cycle;
	int num_pixels = args->framelen/3;

	for (i = 0, j = 0; i < num_pixels; i++, j += 3) {
		int k;
		char* pixel = &args->framebuf[j];
		get_cycle(args, i, &k, &cycle);
		int c = k * RAINBOW_LENGTH / num_pixels;

		if (cycle == 9) {         // left edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(c), args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(c), args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(c), args->shift_remainder);
		} else if (cycle == 14) { // right edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(c), 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(c), 0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(c), 0xff - args->shift_remainder);
		} else if (cycle >= 10) { // middle
			pixel[0] = 0x80 | RAINBOW_G(c) >> 1;
			pixel[1] = 0x80 | RAINBOW_R(c) >> 1;
			pixel[2] = 0x80 | RAINBOW_B(c) >> 1;
		} else {
			pixel[0] = 0x80;
			pixel[1] = 0x80;
			pixel[2] = 0x80;
		}
	}
}

struct effect effect_treads_antialias = {
	.name          = "antialias",
	.arg_desc      = "N/A",
	.render        = antialias_treads,
	.sensor_driven = 1,
};

struct effect effect_treads_rolling_rainbow = {
	.name          = "rolling rainbow",
	.arg_desc      = "N/A",
	.render        = rolling_rainbow_treads,
	.sensor_driven = 1,
};
