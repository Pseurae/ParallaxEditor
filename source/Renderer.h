#pragma once

#include <string>
#include <array>
#include "Utils.h"

struct Renderer final
{
    bool loadedPalettes = false;
    Palette palettes[16];

    unsigned int vertexArray;

    unsigned int pickerVertexBuffer, pickerElementBuffer;
    unsigned int pickerTilesetTex, pickerPaletteTex;
    unsigned int pickerFrameBuffer;
    unsigned int pickerFinalTex;
    unsigned int pickerShader;

    unsigned int mapVertexBuffer, mapElementBuffer;
    unsigned int mapFrameBuffer;
    unsigned int mapPaletteTex;
    unsigned int mapFinalTex;
    unsigned int mapShader;
};

bool renderer_init(Renderer &);
void renderer_load_primary(Renderer &, unsigned char *data);
void renderer_load_secondary(Renderer &, unsigned char *data);
void renderer_load_palette(Renderer &r, int idx, const Palette plt);
void renderer_change_palette(Renderer &, int idx);
void renderer_load_map_palette(Renderer &r);
void renderer_call(Renderer &, unsigned short tilemap[]);
