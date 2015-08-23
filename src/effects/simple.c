#include <stdint.h>
#include <stddef.h>

#include "effects.h"

static
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

static
void simple_panels(struct render_args* args) {
	int i;
	union color color;

	for (i = 0; i < args->framelen; i += 3) {
		color = args->effect->color_arg.colors[i / 3];

		args->framebuf[i+0] = color.rgb.red;
		args->framebuf[i+1] = color.rgb.green;
		args->framebuf[i+2] = color.rgb.blue;
	}
}

struct effect effect_treads_simple = {
	.name          = "simple",
	.arg_desc      = "N/A",
	.render        = simple_treads,
	.sensor_driven = 1,
};

struct effect effect_panels_simple = {
	.name          = "simple",
	.arg_desc      = "N/A",
	.render        = simple_panels,
};
