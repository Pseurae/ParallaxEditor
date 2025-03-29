#pragma once

#include <functional>

struct Tile
{
    unsigned short x, y;
    bool xflip, yflip;
    unsigned short id;
    unsigned char palette;
};


template<>
struct std::hash<Tile>
{
    std::size_t operator()(const Tile& s) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(s.x);
        std::size_t h2 = std::hash<int>{}(s.y);
        return h1 ^ (h2 << 1);
    }
};
