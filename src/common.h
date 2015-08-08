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
	int fd_pru;

	// Dereference to get the current number of encoder ticks
	int* ticks;
};

int pal_init(struct pal* p);
int pal_treads_write(struct pal* p, const char* buf, size_t len);
int pal_barrel_write(struct pal* p, const char* buf, size_t len);
int pal_panels_write(struct pal* p, const char* buf, size_t len);
int pal_destroy(struct pal* p);
