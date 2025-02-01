#include "Utils.h"
#include "Renderer.h"
#include "Global.h"
#include <stb_image.h>

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

static constexpr char sText_JASC_PAL[] = "JASC-PAL";
static constexpr char sText_PAL_0100[] = "0100";

std::vector<Color> load_palette(const std::string &fname)
{
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

    if (num_colors < 1 || num_colors > 256)
        throw ("Unsupported number of colors. (Color count must be between 1 and 256)");

    for (int i = 0; i < num_colors; i++)
    {
        if (!(stream >> r >> g >> b))
            throw ("Error parsing color components.");

        if (r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
            throw ("Color component value must be between 0 and 255.");

        colors.push_back({ 
            static_cast<unsigned char>(r), 
            static_cast<unsigned char>(g), 
            static_cast<unsigned char>(b) 
        });
    }

    stream.close();

    return colors;
}

void load_tilemap(const std::string &fname)
{
    std::ifstream fs(fname, std::ios::binary);

    for (int i = 0; i < 0x400; ++i)
    {
        uint16_t entry;
        fs.read(reinterpret_cast<char *>(&entry), 2);
        global.tilemap[i] = entry;
    }

    fs.close();
}

void load_primary_tileset(const std::string &fname)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &n, 4);

    if (!data || w != 128 || h != 256 || n != 1)
    {
        stbi_image_free(data);
        return;
    }

    renderer_load_primary(global.renderer, data);
    stbi_image_free(data);
}

void load_secondary_tileset(const std::string &fname)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &n, 4);

    if (!data || w != 128 || h != 256 || n != 1)
    {
        stbi_image_free(data);
        return;
    }

    renderer_load_secondary(global.renderer, data);
    stbi_image_free(data);
}


static const char gPaletteFileNames[][7] =
{
    "00.pal",
    "01.pal",
    "02.pal",
    "03.pal",
    "04.pal",
    "05.pal",
    "06.pal",
    "07.pal",
    "08.pal",
    "09.pal",
    "10.pal",
    "11.pal",
    "12.pal",
    "13.pal",
    "14.pal",
    "15.pal"
};

void load_palettes(const std::string &s)
{
    std::filesystem::path basePalettePath = s;

    for (int i = 0; i < 16; ++i)
    {
        auto palPath = basePalettePath / gPaletteFileNames[i];

        if (std::filesystem::exists(palPath))
            renderer_load_palette(global.renderer, i, load_palette(palPath.string()).data());
    }

    renderer_change_palette(global.renderer, global.brush.palette);
}