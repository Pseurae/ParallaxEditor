#pragma once

#include <unordered_set>
#include "Tile.h"

class Tilemap final
{
public:
    Tilemap(unsigned short width, unsigned short height) : mWidth(width), mHeight(height) {}
    ~Tilemap() = default;

    auto &GetTiles() { return mTiles; } 
    const auto &GetTiles() const { return mTiles; } 

private:
    unsigned short mWidth, mHeight;
    Tile mDefault;
    std::unordered_set<Tile> mTiles;
};

