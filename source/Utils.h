#pragma once

struct Color final { unsigned char r, g, b; };
using Palette = Color[16];

#include <vector>
#include <string>

enum Mask : short
{
    Index = 0x3FF,
    FlipX = 0x400,
    FlipY = 0x800
};

void open_tilemap(void);
void save_tilemap(void);
void save_as_tilemap(void);
void open_primary_tileset(void);
void open_secondary_tileset(void);
void open_palettes(void);
