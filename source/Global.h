#pragma once

#include <string>
#include "Renderer.h"

struct Brush
{
    unsigned short tile;
    unsigned char palette;
    bool xflip;
    bool yflip;
};

struct Global
{
    double dpiScale, zoomScale = 1.0f;
    Brush brush;
    Renderer renderer;

    std::string tilemapPath;
    unsigned short tilemap[32 * 32];
};

extern Global global;