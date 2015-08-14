#include <stdint.h>
#include <stddef.h>

#include "effects.h"

static inline
uint8_t scale_color(uint8_t color, uint8_t scale) {
	return ((int)color * (int)(scale)) >> 9;
}

void antialias_treads(struct render_args* args) {
	int i;
	int j;
	int cycle;

	for (i = 0; i < args->framelen/3; i++) {
		char* pixel = &args->framebuf[i*3];
		union color color = args->effect->color_arg.color;
		get_cycle(args, i, &j, &cycle);

		if (cycle == 9) {         // left edge
			pixel[0] = 0x80 | scale_color(color.rgb.green, args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.red, args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue, args->shift_remainder);
		} else if (cycle == 14) { // right edge
			pixel[0] = 0x80 | scale_color(color.rgb.green, 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.red, 0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue, 0xff - args->shift_remainder);
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

void rolling_rainbow_treads(struct render_args* args) {
	int i;
	int j;
	int cycle;

	for (i = 0; i < args->framelen/3; i++) {
		char* pixel = &args->framebuf[i*3];
		get_cycle(args, i, &j, &cycle);

		if (cycle == 9) {         // left edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(j), args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(j), args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(j), args->shift_remainder);
		} else if (cycle == 14) { // right edge
			pixel[0] = 0x80 | scale_color(RAINBOW_G(j), 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(RAINBOW_R(j), 0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(RAINBOW_B(j), 0xff - args->shift_remainder);
		} else if (cycle >= 10) { // middle
			pixel[0] = 0x80 | RAINBOW_G(j) >> 1;
			pixel[1] = 0x80 | RAINBOW_R(j) >> 1;
			pixel[2] = 0x80 | RAINBOW_B(j) >> 1;
		} else {
			pixel[0] = 0x80;
			pixel[1] = 0x80;
			pixel[2] = 0x80;
		}
	}
}
