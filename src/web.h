#pragma once

int web_init(struct engine* eng, const char* port);
void web_run();
void web_destroy();

void web_treads_render(const char* buf, size_t len);
void web_barrel_render(const char* buf, size_t len);
