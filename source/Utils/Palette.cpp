#include "Palette.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

Palette::Palette()
{
    mColors.fill({ 0, 0, 0 });
}

Palette::Palette(const std::string &fname)
{
    Load(fname);
}

void Palette::Load(const std::string &fname)
{
    static constexpr char sText_JASC_PAL[] = "JASC-PAL";
    static constexpr char sText_PAL_0100[] = "0100";
    
    std::ifstream stream(fname);
    std::vector<Color> colors;
    std::string line;

    uint16_t num_colors;
    int r, g, b;

    stream >> line;
    if (line != sText_JASC_PAL) 
        throw ("Invalid JASC-PAL signature.");

    stream >> line;
    if (line != sText_PAL_0100) 
        throw ("Unsupported JASC-PAL version.");

    if (!(stream >> num_colors))
        throw ("Could not parse number of colors.");

    if (num_colors != 16)
        throw ("Unsupported number of colors. (Color count must be between 1 and 256)");

    for (int i = 0; i < 16; i++)
    {
        if (!(stream >> r >> g >> b))
            throw ("Error parsing color components.");

        if (r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
            throw ("Color component value must be between 0 and 255.");

        mColors[i] = { 
            static_cast<unsigned char>(r), 
            static_cast<unsigned char>(g), 
            static_cast<unsigned char>(b) 
        };
    }
}