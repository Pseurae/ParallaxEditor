#pragma once

#include "Utils/Tile.h"
#include <vector>

struct Brush
{
    unsigned int tile;
    bool xflip;
    bool yflip;
    bool scrollToSelected;
};