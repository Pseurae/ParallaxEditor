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
    Tile GetDefaultTile() const { return mDefaultTile; }

private:
    unsigned short mWidth, mHeight;
    Tile mDefaultTile;
    std::unordered_set<Tile> mTiles;
};

