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

	// Wheel encoder: dereference to get the current values
	volatile unsigned int* enc_timer; // Number of ADC reads
	volatile unsigned int* enc_raw;   // Raw ADC value
	volatile unsigned int* enc_min;   // Min value for current half-tick
	volatile unsigned int* enc_max;   // Max value for current half-tick
	volatile unsigned int* enc_ticks; // Number of encoder ticks
	volatile unsigned int* enc_speed; // Width of last encoder tick
};

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay);

int pal_treads_write(const char* buf, size_t len);
int pal_barrel_write(const char* buf, size_t len);
int pal_panels_write(const char* buf, size_t len);

void pal_destroy();

int pal_clock_gettime(struct timespec* tv);
