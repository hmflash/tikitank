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

	for (i = 0; i < num_pixels; i++) {
		int k;
		char* pixel = &args->framebuf[i*3];

		get_cycle(args, i, &j, &cycle);

		k = j * args->framelen / RAINBOW_LENGTH;

		if (cycle == 9) {         // left edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(k), args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(k), args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(k), args->shift_remainder);
		} else if (cycle == 14) { // right edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(k), 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(k), 0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(k), 0xff - args->shift_remainder);
		} else if (cycle >= 10) { // middle
			pixel[0] = 0x80 | RAINBOW_G(k) >> 1;
			pixel[1] = 0x80 | RAINBOW_R(k) >> 1;
			pixel[2] = 0x80 | RAINBOW_B(k) >> 1;
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
