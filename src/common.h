#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define LOG(a) debug_log a

void debug_log(const char* fmt, ...)
	__attribute__((format(printf, 1, 2)));

struct pal {
	// Brightness of panels from 1-8
	int panel_brightness;

	// Wheel encoder: dereference to get the current values
	unsigned int* enc_timer; // Number of ADC reads
	unsigned int* enc_raw;   // Raw ADC value
	unsigned int* enc_min;   // Min value for current half-tick
	unsigned int* enc_max;   // Max value for current half-tick
	unsigned int* enc_ticks; // Number of encoder ticks
	unsigned int* enc_speed; // Width of last encoder tick
};

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay);

int pal_treads_write(const char* buf, size_t len);
int pal_barrel_write(const char* buf, size_t len);
int pal_panels_write(const char* buf, size_t len);

void pal_destroy();

int pal_clock_gettime(struct timespec* tv);
