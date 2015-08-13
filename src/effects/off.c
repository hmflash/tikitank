#include <string.h>

#include "effects.h"

void off_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	memset(framebuf, 0x80, framelen);
}

void off_barrel(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	memset(framebuf, 0x80, framelen);
}

void off_panels(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen) {
	memset(framebuf, 0, framelen);
}
