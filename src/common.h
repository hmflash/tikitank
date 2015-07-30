#pragma once

#include <stdint.h>

#define LOG(a) debug_log a

extern uint8_t fastled_rainbow[256][3];

void debug_log(const char const* fmt, ...)
	__attribute__((format(printf, 1, 2)));
