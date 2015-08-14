#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void rainbow_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	int i;

	for (i = 0; i < framelen; i += 3) {
		framebuf[i+0] = 0x80 | RAINBOW_G(i + (4 * framenum)) >> 1;
		framebuf[i+1] = 0x80 | RAINBOW_R(i + (4 * framenum)) >> 1;
		framebuf[i+2] = 0x80 | RAINBOW_B(i + (4 * framenum)) >> 1;
	}
}

void rainbow_barrel(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	int i;

	for (i = 0; i < framelen; i += 3) {
		framebuf[i+0] = 0x80 | RAINBOW_G(i + (4 * framenum)) >> 1;
		framebuf[i+1] = 0x80 | RAINBOW_R(i + (4 * framenum)) >> 1;
		framebuf[i+2] = 0x80 | RAINBOW_B(i + (4 * framenum)) >> 1;
	}
}

void rainbow_panels(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	int i;

	for (i = 0; i < framelen; i += 3) {
		framebuf[i+0] = CC_R(RAINBOW_R(framenum));
		framebuf[i+1] = CC_G(RAINBOW_G(framenum));
		framebuf[i+2] = CC_B(RAINBOW_B(framenum));
	}
}
