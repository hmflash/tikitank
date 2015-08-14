#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void simple_treads(struct render_args* args) {
	int i;
	int j;
	int cycle;

	for (i = 0; i < args->framelen/3; i++) {
		char* pixel = &args->framebuf[i*3];
		union color color = args->effect->color_arg.color;
		get_cycle(args, i, &j, &cycle);

		if (cycle >= 10) {
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

void simple_panels(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	int i;
	union color color;

	for (i = 0; i < framelen; i += 3) {
		color = effect->color_arg.colors[i / 3];

		framebuf[i+0] = color.rgb.red;
		framebuf[i+1] = color.rgb.green;
		framebuf[i+2] = color.rgb.blue;
	}
}
