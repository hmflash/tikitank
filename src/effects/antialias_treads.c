#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void antialias_treads(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		int cycle = (args->framelen - i + args->shift_quotient*3) % (15*3);

		if (cycle >= (10*3)) {
			args->framebuf[i+0] = args->effect->color_arg.color.rgb.red;
			args->framebuf[i+1] = args->effect->color_arg.color.rgb.green;
			args->framebuf[i+2] = args->effect->color_arg.color.rgb.blue;
		} else {
			args->framebuf[i+0] = 0x80;
			args->framebuf[i+1] = 0x80;
			args->framebuf[i+2] = 0x80;
		}
	}
}
