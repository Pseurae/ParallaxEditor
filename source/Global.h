#pragma once

#include <string>
#include "Renderer.h"

struct Brush
{
    std::vector<unsigned short> selection{0};
    int width{1}, height{1};

    bool fromTileset = true;

    unsigned char palette;
    bool xflip;
    bool yflip;
};

struct Global
{
    double dpiScale, zoomScale = 1.0f;
    Brush brush;
    Renderer renderer;

    bool drawScreenBounds = false;

    std::string tilemapPath;
    unsigned short tilemap[32 * 32];
};

extern Global global;