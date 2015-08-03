#pragma once

struct engine;
struct pal;

struct engine* engine_init(struct pal* pal);
int engine_run();
int engine_destroy();
