#pragma once

#include "Utils/Tile.h"
#include <vector>

struct Brush
{
    std::vector<Tile> selection{Tile{false, false, 0, 0}};
    unsigned int width = 1, height = 1;
    bool fromTileset = true, xflip = false, yflip = false, scrollToSelected;
};