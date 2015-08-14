#include <stdint.h>
#include <stddef.h>

#include "effects.h"

static inline
uint8_t scale_color(uint8_t color, uint8_t scale) {
	return ((int)color * (int)(scale)) >> 9;
}

void antialias_treads(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen/3; i++) {
		char* pixel = &args->framebuf[i*3];
		union color color = args->effect->color_arg.color;
		int cycle = (args->framelen + args->shift_quotient - i) % 15;

		if (cycle == 9) {         // left edge
			pixel[0] = 0x80 | scale_color(color.rgb.red, args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.green, args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue, args->shift_remainder);
		} else if (cycle == 14) { // right edge
			pixel[0] = 0x80 | scale_color(color.rgb.red, 0xff - args->shift_remainder);
			pixel[1] = 0x80 | scale_color(color.rgb.green, 0xff - args->shift_remainder);
			pixel[2] = 0x80 | scale_color(color.rgb.blue, 0xff - args->shift_remainder);
		} else if (cycle >= 10) { // middle
			pixel[0] = 0x80 | color.rgb.red;
			pixel[1] = 0x80 | color.rgb.green;
			pixel[2] = 0x80 | color.rgb.blue;
		} else {
			pixel[0] = 0x80;
			pixel[1] = 0x80;
			pixel[2] = 0x80;
		}
	}
}