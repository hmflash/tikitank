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

struct effect effect_treads_off = {
	.name          = "off",
	.arg_desc      = "N/A",
	.render        = off_treads,
};

struct effect effect_panels_off = {
	.name          = "off",
	.arg_desc      = "N/A",
	.render        = off_panels,
};

struct effect effect_barrel_off = {
	.name          = "off",
	.arg_desc      = "N/A",
	.render        = off_barrel,
};

