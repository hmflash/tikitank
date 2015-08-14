#include <string.h>

#include "effects.h"

void off_treads(struct render_args* args) {
	memset(args->framebuf, 0x80, args->framelen);
}

void off_barrel(struct render_args* args) {
	memset(args->framebuf, 0x80, args->framelen);
}

void off_panels(struct render_args* args) {
	memset(args->framebuf, 0, args->framelen);
}
