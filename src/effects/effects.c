#include <stddef.h>

#include "effects.h"

void rainbow_treads(struct effect* effect, int framenum, char* framebuf, size_t framelen);

struct effect effect_rainbow = {
	.name          = "rainbow",
	.arg_desc      = "N/A",
	.render_treads = rainbow_treads,
	.render_barrel = NULL,
	.render_panels = NULL,
};

struct effect effect_off = {
	.name          = "off",
	.arg_desc      = "",
	.render_treads = NULL,
	.render_barrel = NULL,
	.render_panels = NULL,
};

struct effect* effects_table[] = {
	&effect_off,
	&effect_rainbow,
	NULL
};
