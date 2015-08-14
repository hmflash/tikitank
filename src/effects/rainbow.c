#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void rainbow_treads(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		args->framebuf[i+0] = 0x80 | RAINBOW_R(i + args->framenum) >> 1;
		args->framebuf[i+1] = 0x80 | RAINBOW_G(i + args->framenum) >> 1;
		args->framebuf[i+2] = 0x80 | RAINBOW_B(i + args->framenum) >> 1;
	}
}

void rainbow_panels(struct render_args* args) {
	int i;

	for (i = 0; i < args->framelen; i += 3) {
		args->framebuf[i+0] = RAINBOW_R(args->framenum);
		args->framebuf[i+1] = RAINBOW_G(args->framenum);
		args->framebuf[i+2] = RAINBOW_B(args->framenum);
	}
}
