#pragma once

#include <functional>

struct Tile
{
    bool xflip = false, yflip = false;
    unsigned short id;
    unsigned char palette;
};

struct TilePosition
{
    unsigned int x, y;

    constexpr bool operator==(const TilePosition &pos) const
    {
        return pos.x == x && pos.y == y;
    }
};


template<>
struct std::hash<TilePosition>
{
    std::size_t operator()(const TilePosition& s) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(s.x);
        std::size_t h2 = std::hash<int>{}(s.y);
        return h1 ^ (h2 << 1);
    }
};
