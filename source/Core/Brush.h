#pragma once

#include "Utils/Tile.h"
#include <vector>

struct Brush
{
    std::vector<Tile> selection;
    unsigned int width, height;
    bool fromTileset, xflip, yflip, scrollToSelected;
};