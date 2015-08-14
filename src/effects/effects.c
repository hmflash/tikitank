#include <stddef.h>

#include "effects.h"

void off_treads(struct render_args* args);
void off_barrel(struct render_args* args);
void off_panels(struct render_args* args);

void simple_treads(struct render_args* args);
void simple_panels(struct render_args* args);
void antialias_treads(struct render_args* args);

void rainbow_treads(struct render_args* args);
void rainbow_barrel(struct render_args* args);
void rainbow_panels(struct render_args* args);

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

struct effect effect_treads_antialias = {
	.name       = "antialias",
	.arg_desc   = "N/A",
	.render     = antialias_treads,
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

struct effect effect_barrel_rainbow = {
	.name       = "rainbow",
	.arg_desc   = "N/A",
	.render     = rainbow_barrel,
};

struct effect effect_panels_off = {
	.name       = "off",
	.arg_desc   = "N/A",
	.render     = off_panels,
};

struct effect effect_panels_simple = {
	.name       = "simple",
	.arg_desc   = "N/A",
	.render     = simple_panels,
};

struct effect effect_panels_rainbow = {
	.name       = "rainbow",
	.arg_desc   = "N/A",
	.render     = rainbow_panels,
};

struct effect* effects_treads[] = {
	&effect_treads_off,
	&effect_treads_simple,
	&effect_treads_antialias,
	&effect_treads_rainbow,
};

struct effect* effects_barrel[] = {
	&effect_barrel_off,
	&effect_barrel_rainbow,
};

struct effect* effects_panels[] = {
	&effect_panels_off,
	&effect_panels_simple,
	&effect_panels_rainbow,
};

struct channel channel_treads = {
	.active      = 0,
	.num_effects = sizeof(effects_treads) / sizeof(*effects_treads),
	.effects     = effects_treads,
};

struct channel channel_barrel = {
	.active      = 0,
	.num_effects = sizeof(effects_barrel) / sizeof(*effects_barrel),
	.effects     = effects_barrel,
};

struct channel channel_panels = {
	.active      = 0,
	.num_effects = sizeof(effects_panels) / sizeof(*effects_panels),
	.effects     = effects_panels,
};
