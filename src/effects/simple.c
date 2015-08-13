#include <stdint.h>
#include <stddef.h>

#include "effects.h"

void simple_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	int i;

	for (i = 0; i < framelen; i += 3) {
		int cycle = (framelen - i + shift*3) % (15*3);

		if (cycle >= (10*3)) {
			framebuf[i+0] = 0xff;
			framebuf[i+1] = 0xff;
			framebuf[i+2] = 0xff;
		} else {
			framebuf[i+0] = 0x80;
			framebuf[i+1] = 0x80;
			framebuf[i+2] = 0x80;
		}
	}
}
