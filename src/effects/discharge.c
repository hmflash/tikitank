#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "effects.h"
#include "common.h"

static
void discharge(struct render_args* args) {
	int i;
	int j;
	union color color = args->effect->color_arg.color;
	int num_pixels = args->framelen/3;
	int rate = args->effect->argument ? args->effect->argument : 1;
	int cycle = (rate * args->framenum) % (num_pixels * 2);
	double t = (double)cycle / (double)num_pixels;
	int x = num_pixels * (t*t);

	for (i = 0, j = 0; i < num_pixels; i++, j += 3) {
		char* pixel = &args->framebuf[j];
		if (i >= x && i < x+5 && i < num_pixels) {
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

struct effect effect_barrel_discharge = {
	.name          = "discharge",
	.arg_desc      = "discharge rate",
	.arg_default   = 2,
	.render        = discharge,
};
