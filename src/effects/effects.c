#include <stddef.h>

#include "effects.h"

inline
void get_cycle(struct render_args* args, int i, int* j, int* cycle) {
	*j = args->framelen + args->shift_quotient - i;
	*cycle = *j % 15;
}

struct effect effect_treads_off;
struct effect effect_treads_simple;
struct effect effect_treads_antialias;
struct effect effect_treads_rolling_rainbow;
struct effect effect_treads_camera_flash;
struct effect effect_treads_rainbow;

struct effect effect_barrel_off;
struct effect effect_barrel_camera_flash;
struct effect effect_barrel_rainbow;

struct effect effect_panels_off;
struct effect effect_panels_simple;
struct effect effect_panels_rainbow;

struct effect* effects_treads[] = {
	// &effect_treads_off,
	&effect_treads_antialias,
	&effect_treads_simple,
	&effect_treads_rolling_rainbow,
	&effect_treads_rainbow,
	&effect_treads_camera_flash,
};

struct effect* effects_barrel[] = {
	// &effect_barrel_off,
	&effect_barrel_rainbow,
	&effect_barrel_camera_flash,
};

struct effect* effects_panels[] = {
	// &effect_panels_off,
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
