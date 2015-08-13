#include <stddef.h>

#include "effects.h"

void off_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);
void off_barrel(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);
void off_panels(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);

void simple_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);

void rainbow_treads(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);
void rainbow_panels(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);

struct effect effect_treads_off = {
	.name       = "off",
	.arg_desc   = "N/A",
	.render     = off_treads,
};

struct effect effect_treads_simple = {
	.name       = "simple",
	.arg_desc   = "N/A",
	.render     = simple_treads,
};

struct effect effect_treads_rainbow = {
	.name       = "rainbow",
	.arg_desc   = "N/A",
	.render     = rainbow_treads,
};

struct effect effect_barrel_off = {
	.name       = "off",
	.arg_desc   = "N/A",
	.render     = off_barrel,
};

struct effect effect_panels_off = {
	.name       = "off",
	.arg_desc   = "N/A",
	.render     = off_panels,
};

struct effect effect_panels_rainbow = {
	.name       = "rainbow",
	.arg_desc   = "N/A",
	.render     = rainbow_panels,
};

struct effect* effects_treads[] = {
	&effect_treads_off,
	&effect_treads_simple,
	&effect_treads_rainbow,
	NULL
};

struct effect* effects_barrel[] = {
	&effect_barrel_off,
	NULL
};

struct effect* effects_panels[] = {
	&effect_panels_off,
	&effect_panels_rainbow,
	NULL
};

int effects_treads_num = sizeof(effects_treads) / sizeof(*effects_treads);
int effects_barrel_num = sizeof(effects_barrel) / sizeof(*effects_barrel);
int effects_panels_num = sizeof(effects_panels) / sizeof(*effects_panels);
