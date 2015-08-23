#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "common.h"

static
int last_frame = -1;

static
void rainbow_flash(struct render_args* args) {
	int i;
	int j;
	int c = args->framenum / 8;
	int num_pixels = args->framelen/3;
	int delta = args->framenum - last_frame;
	int flash_rate = args->effect->argument ? args->effect->argument : 1;
	int flash_quotient  = flash_rate / FRAME_PER_SEC;
	int flash_remainder = flash_rate % FRAME_PER_SEC;

	if (delta > 1) {
		memset(args->framebuf, 0x80, args->framelen);
	}

	for (i = 0; i < args->framelen; i++) {
		int li = (i-3) % args->framelen;
		int mi = (i+0) % args->framelen;
		int ri = (i+3) % args->framelen;
		uint8_t lp = args->framebuf[li] & 0x7f;
		uint8_t mp = args->framebuf[mi] & 0x7f;
		uint8_t rp = args->framebuf[ri] & 0x7f;
		mp = (lp + mp + rp) / 4;
		args->framebuf[mi] = 0x80 | (mp & 0x7f);
	}

	for (j = 0; j < flash_quotient; j++) {
		int p = random() % num_pixels;
		char* pixel = &args->framebuf[p*3];
		pixel[0] = 0x80 | RAINBOW_G(c) >> 1;
		pixel[1] = 0x80 | RAINBOW_R(c) >> 1;
		pixel[2] = 0x80 | RAINBOW_B(c) >> 1;
	}

	if (flash_remainder && !(args->framenum % flash_remainder)) {
		int p = random() % num_pixels;
		char* pixel = &args->framebuf[p*3];
		pixel[0] = 0x80 | RAINBOW_G(c) >> 1;
		pixel[1] = 0x80 | RAINBOW_R(c) >> 1;
		pixel[2] = 0x80 | RAINBOW_B(c) >> 1;
	}

	last_frame = args->framenum;
}

struct effect effect_treads_rainbow_sparkle = {
	.name          = "rainbow sparkle",
	.arg_desc      = "sparkle rate",
	.arg_default   = 500,
	.render        = rainbow_flash,
};

struct effect effect_barrel_rainbow_sparkle = {
	.name          = "rainbow sparkle",
	.arg_desc      = "sparkle rate",
	.arg_default   = 100,
	.render        = rainbow_flash,
};
