#include <stdint.h>
#include <stddef.h>

#include "effects.h"

#define NUM_TREADS (3*32*5*3)

extern
uint8_t fastled_rainbow[256][3];

void rainbow_treads(struct effect* effect, int framenum, char* framebuf, size_t framelen) {
	int i;

	for (i = 0; i < NUM_TREADS; i += 3) {
		framebuf[i+0] = 0x80 | fastled_rainbow[(i + framenum) % 256][1] >> 1;
		framebuf[i+1] = 0x80 | fastled_rainbow[(i + framenum) % 256][0] >> 1;
		framebuf[i+2] = 0x80 | fastled_rainbow[(i + framenum) % 256][2] >> 1;
	}
}
