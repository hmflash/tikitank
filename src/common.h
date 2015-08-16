#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define LOG(a) debug_log a
#define DEBUG_LOG(a) if (verbosity > 0) debug_log a
#define TRACE_LOG(a) if (verbosity > 1) debug_log a

extern int verbosity;

void debug_log(const char* fmt, ...)
	__attribute__((format(printf, 1, 2)));

struct pal {
	// Brightness of panels from 1-8
	int panel_brightness;

	// Frame buffer for effects
	char* treads_buf; // Length: NUM_TREADS
	char* barrel_buf; // Length: NUM_BARREL
	char* panels_buf; // Length: NUM_PANELS

	// Wheel encoder: dereference to get the current values
	volatile unsigned int* enc_timer; // Number of ADC reads
	volatile unsigned int* enc_raw;   // Raw ADC value
	volatile unsigned int* enc_min;   // Min value for current half-tick
	volatile unsigned int* enc_max;   // Max value for current half-tick
	volatile unsigned int* enc_ticks; // Number of encoder ticks
	volatile unsigned int* enc_speed; // Width of last encoder tick
};

struct settings {
	long                brightness;
	long                manual_tick;
	long                idle_interval;
};

extern struct settings settings;

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay);

void pal_treads_write();
void pal_barrel_write();
void pal_panels_write();

void pal_destroy();

int pal_clock_gettime(struct timespec* tv);

int settings_load();
