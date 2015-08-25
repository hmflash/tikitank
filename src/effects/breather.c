#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "effects.h"
#include "common.h"

// new = ((old - old_min) / (old_max - old_min)) * (new_max - new_min) + new_min
// old_min = 0
// old_max = rate
// new_min = 0
// new_max = M_2_PI
// new = (f / rate) * M_2_PI

static inline
int scale_color(int c, double scale, int shift) {
	int nonzeroscale = (scale != 0) ? 1 : 0;
	return (c == 0) ? 0 : ((c >> shift) * scale) + nonzeroscale;
}

static
void breather_treads(struct render_args* args) {
	int i;
	int num_pixels = args->framelen/3;
	union color color = args->effect->color_arg.color;
	double rate = args->effect->argument ? args->effect->argument : 1.0;
	double c = (sin((args->framenum / rate) * M_2_PI) / 2.0) + 0.5;
	union color scaled = {
		.rgb = {
			.red   = scale_color(color.rgb.red,   c, 1),
			.green = scale_color(color.rgb.green, c, 1),
			.blue  = scale_color(color.rgb.blue,  c, 1),
		}
	};

	for (i = 0; i < num_pixels; i++) {
		char* pixel = &args->framebuf[i*3];
		pixel[0] = 0x80 | scaled.rgb.green;
		pixel[1] = 0x80 | scaled.rgb.red;
		pixel[2] = 0x80 | scaled.rgb.blue;
	}
}

static
void breather_panels(struct render_args* args) {
	int i;
	double rate = args->effect->argument ? args->effect->argument : 1.0;
	double c = (sin((args->framenum / rate) * M_2_PI) / 2.0) + 0.5;

	for (i = 0; i < args->framelen; i += 3) {
		char* pixel = &args->framebuf[i];
		union color color = args->effect->color_arg.colors[i / 3];
		union color scaled = {
			.rgb = {
				.red   = scale_color(color.rgb.red,   c, 0),
				.green = scale_color(color.rgb.green, c, 0),
				.blue  = scale_color(color.rgb.blue,  c, 0),
			}
		};

		pixel[0] = scaled.rgb.red;
		pixel[1] = scaled.rgb.green;
		pixel[2] = scaled.rgb.blue;
	}
}

struct effect effect_treads_breather = {
	.name          = "breather",
	.arg_desc      = "cycle rate",
	.arg_default   = 100,
	.render        = breather_treads,
};

struct effect effect_barrel_breather = {
	.name          = "breather",
	.arg_desc      = "cycle rate",
	.arg_default   = 100,
	.render        = breather_treads,
};

struct effect effect_panels_breather = {
	.name          = "breather",
	.arg_desc      = "cycle rate",
	.arg_default   = 100,
	.render        = breather_panels,
};
