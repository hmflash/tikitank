#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "common.h"

static
int last_frame = -1;

#define MAX(x, y) (x >= y) ? (x) : (y)

static
void camera_flash(struct render_args* args) {
	int i;
	int j;
	union color color = args->effect->color_arg.color;
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
		pixel[0] = 0x80 | color.rgb.green;
		pixel[1] = 0x80 | color.rgb.red;
		pixel[2] = 0x80 | color.rgb.blue;
	}

	if (flash_remainder && !(args->framenum % flash_remainder)) {
		int p = random() % num_pixels;
		char* pixel = &args->framebuf[p*3];
		pixel[0] = 0x80 | color.rgb.green;
		pixel[1] = 0x80 | color.rgb.red;
		pixel[2] = 0x80 | color.rgb.blue;
	}

	last_frame = args->framenum;
}

struct effect effect_treads_camera_flash = {
	.name          = "camera flash",
	.arg_desc      = "flash rate",
	.arg_default   = 8,
	.render        = camera_flash,
};

struct effect effect_barrel_camera_flash = {
	.name          = "camera flash",
	.arg_desc      = "flash rate",
	.arg_default   = 8,
	.render        = camera_flash,
};
