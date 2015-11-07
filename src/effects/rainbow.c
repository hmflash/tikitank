#include <stdint.h>
#include <stddef.h>

#include "effects.h"

// new = ((old - old_min) / (old_max - old_min)) * (new_max - new_min) + new_min
// old_min = 0
// old_max = RAINBOW_LENGTH
// new_min = 0
// new_max = framelen
// new = (i / RAINBOW_LENGTH) * framelen

static
void rainbow(struct render_args* args) {
	int i;
	int j;
	int num_pixels = args->framelen/3;
	int rate = args->effect->argument ? args->effect->argument : 1;

	for (i = 0, j = 0; i < num_pixels; i++, j += 3) {
		char* pixel = &args->framebuf[j];
		int k = (num_pixels - i) * RAINBOW_LENGTH / num_pixels;
		int c = rate * args->framenum + k;
		pixel[0] = 0x80 | RAINBOW_G(c) >> 1;
		pixel[1] = 0x80 | RAINBOW_R(c) >> 1;
		pixel[2] = 0x80 | RAINBOW_B(c) >> 1;
	}
}

static
void rainbow_panels(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		args->framebuf[i+0] = RAINBOW_R(args->framenum);
		args->framebuf[i+1] = RAINBOW_G(args->framenum);
		args->framebuf[i+2] = RAINBOW_B(args->framenum);
	}
}

static
void rolling_rainbow_panels(struct render_args* args) {
	int i;
	int j;
	int scale = RAINBOW_LENGTH / NUM_PANELS;

	for (i = 0, j = 0; j < args->framelen; i++, j += 3) {
		char* pixel = &args->framebuf[j];
		int c = args->framenum + i * scale;
		pixel[0] = RAINBOW_R(c);
		pixel[1] = RAINBOW_G(c);
		pixel[2] = RAINBOW_B(c);
	}
}

struct effect effect_treads_rainbow = {
	.name          = "rainbow",
	.arg_desc      = "cycle rate",
	.arg_default   = 10,
	.render        = rainbow,
};

struct effect effect_barrel_rainbow = {
	.name          = "rainbow",
	.arg_desc      = "cycle rate",
	.arg_default   = 20,
	.render        = rainbow,
};

struct effect effect_panels_rainbow = {
	.name          = "rainbow",
	.arg_desc      = "N/A",
	.render        = rainbow_panels,
};

struct effect effect_panels_rolling_rainbow = {
	.name          = "rolling rainbow",
	.arg_desc      = "N/A",
	.render        = rolling_rainbow_panels,
};
