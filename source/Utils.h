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

std::vector<Color> load_palette(const std::string &fname);
void load_tilemap(const std::string &fname);
void load_primary_tileset(const std::string &fname);
void load_secondary_tileset(const std::string &fname);
void load_palettes(const std::string &s);

void open_tilemap(void);
void open_primary_tileset(void);
void open_secondary_tileset(void);
void open_palettes(void);
