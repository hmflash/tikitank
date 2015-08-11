#pragma once

#include <stddef.h>
#include <stdint.h>

#define LOG(a) debug_log a

extern
uint8_t fastled_rainbow[256][3];

void debug_log(const char const* fmt, ...)
	__attribute__((format(printf, 1, 2)));

struct pal {
	int fd_treads;
	int fd_barrel;
	int fd_panels;

	// Wheel encoder: dereference to get the current values
	unsigned int* enc_timer; // Number of ADC reads
	unsigned int* enc_raw;   // Raw ADC value
	unsigned int* enc_min;   // Min value for current half-tick
	unsigned int* enc_max;   // Max value for current half-tick
	unsigned int* enc_ticks; // Number of encoder ticks
	unsigned int* enc_speed; // Width of last encoder tick
};

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay);

int pal_treads_write(struct pal* p, const char* buf, size_t len);
int pal_barrel_write(struct pal* p, const char* buf, size_t len);
int pal_panels_write(struct pal* p, const char* buf, size_t len);

void pal_destroy();
