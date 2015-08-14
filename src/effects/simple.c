#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void simple_treads(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		int cycle = (args->framelen - i + args->shift_quotient*3) % (15*3);

		if (cycle >= (10*3)) {
			args->framebuf[i+0] = 0x80 | args->effect->color_arg.color.rgb.green;
			args->framebuf[i+1] = 0x80 | args->effect->color_arg.color.rgb.red;
			args->framebuf[i+2] = 0x80 | args->effect->color_arg.color.rgb.blue;
		} else {
			args->framebuf[i+0] = 0x80;
			args->framebuf[i+1] = 0x80;
			args->framebuf[i+2] = 0x80;
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
