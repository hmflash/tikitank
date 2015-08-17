#include <stdint.h>
#include <stddef.h>

#include "effects.h"

// new = ((old - old_min) / (old_max - old_min)) * (new_max - new_min) + new_min
// old_min = 0
// old_max = RAINBOW_LENGTH
// new_min = 0
// new_max = framelen
// new = (i / RAINBOW_LENGTH) * framelen

void rainbow_treads(struct render_args* args) {
	int i;
	int speed = 4 * args->framenum;

	for (i = 0; i < args->framelen; i += 3) {
		int index = args->framelen - i + args->framenum + speed;
		int scaled = index * args->framelen / RAINBOW_LENGTH / 4;
		args->framebuf[i+0] = 0x80 | RAINBOW_G(scaled) >> 1;
		args->framebuf[i+1] = 0x80 | RAINBOW_R(scaled) >> 1;
		args->framebuf[i+2] = 0x80 | RAINBOW_B(scaled) >> 1;
	}
}

void rainbow_barrel(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		args->framebuf[i+0] = 0x80 | RAINBOW_G(i + (4 * args->framenum)) >> 1;
		args->framebuf[i+1] = 0x80 | RAINBOW_R(i + (4 * args->framenum)) >> 1;
		args->framebuf[i+2] = 0x80 | RAINBOW_B(i + (4 * args->framenum)) >> 1;
	}
}

void rainbow_panels(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		args->framebuf[i+0] = CC_R(RAINBOW_R(args->framenum));
		args->framebuf[i+1] = CC_G(RAINBOW_G(args->framenum));
		args->framebuf[i+2] = CC_B(RAINBOW_B(args->framenum));
	}
}
