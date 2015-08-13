#pragma once

#include <stdint.h>

// Reasonable color correction values from
// https://github.com/FastLED/FastLED/blob/master/color.h
// TypicalSMD5050   = ( 255, 176, 240 )
// UncorrectedColor = ( 255, 255, 255 )

#define COLOR_CORRECT(pixel, scale) ( (pixel * (scale + 1)) >> 8 )

#define CC_R(pixel) COLOR_CORRECT(pixel, 255)
#define CC_G(pixel) COLOR_CORRECT(pixel, 176)
#define CC_B(pixel) COLOR_CORRECT(pixel, 240)

#define NUM_TREADS     (3*32*5*3)
#define NUM_BARREL      10*3  // FIXME
#define NUM_PANELS      10

#define RAINBOW_LENGTH 768

extern char fastled_rainbow[RAINBOW_LENGTH][3];
extern char linear_rainbow[RAINBOW_LENGTH][3];

#define RAINBOW_R(idx) fastled_rainbow[(idx) % RAINBOW_LENGTH][0]
#define RAINBOW_G(idx) fastled_rainbow[(idx) % RAINBOW_LENGTH][1]
#define RAINBOW_B(idx) fastled_rainbow[(idx) % RAINBOW_LENGTH][2]

union color {
	struct {
		uint8_t         red;
		uint8_t         green;
		uint8_t         blue;
		uint8_t         alpha;         // unused
	} rgb;
	long                value;
};

union color_arg {
	union color         color;
	union color         colors[NUM_PANELS];
};

struct effect {
	const char*         name;
	long                argument;
	const char*         arg_desc;
	union color_arg     color_arg;
	long                screen_saver;
	long                sensor_driven;

	void (*render)(struct effect* effect, int shift, int framenum, char* framebuf, size_t framelen);
};

extern struct effect* effects_treads[];
extern int effects_treads_num;

extern struct effect* effects_barrel[];
extern int effects_barrel_num;

extern struct effect* effects_panels[];
extern int effects_panels_num;
