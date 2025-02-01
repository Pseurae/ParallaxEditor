#pragma once

struct Renderer;

void renderer_map_init(Renderer &r);
void renderer_call_map(Renderer &r, unsigned short tilemap[]);