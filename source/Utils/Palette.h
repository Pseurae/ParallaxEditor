#pragma once

#include <string>
#include <array>

struct Color final { unsigned char r, g, b; };

class Palette final
{
public:
    Palette();
    Palette(const std::string &fname);
    void Load(const std::string &fname);
    const auto &GetColors() const { return mColors; }
private:
    std::array<Color, 16> mColors;
};