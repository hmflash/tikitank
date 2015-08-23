#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "common.h"

static
void ruler(struct render_args* args) {
	int i = args->effect->argument % (args->framelen / 3);
	union color color = args->effect->color_arg.color;

	memset(args->framebuf, 0x80, args->framelen);

	args->framebuf[i*3+0] = 0x80 | color.rgb.green;
	args->framebuf[i*3+1] = 0x80 | color.rgb.red;
	args->framebuf[i*3+2] = 0x80 | color.rgb.blue;
}

struct effect effect_treads_ruler = {
	.name          = "ruler",
	.arg_desc      = "pixel offset",
	.render        = ruler,
};
