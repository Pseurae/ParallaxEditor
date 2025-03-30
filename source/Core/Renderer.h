#pragma once

#include <string>
#include "Utils/Palette.h"
#include "Utils/Tile.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

class Context;

class Renderer final
{
public:
    struct Texture
    {
        unsigned int width, height, id;
    };

    struct RenderTarget
    {
        unsigned fbo;
        Texture tex;
    };

    void Initialize(void);
    void Shutdown(void);

    const Texture &GetPickerTex(unsigned int i) const { return mPickerTexes[i].tex; }
    const Texture &GetMapTex() const { return mMapTex.tex; }

    void LoadPrimaryTileset(const std::string &fname);
    void LoadSecondaryTileset(const std::string &fname);
    void Draw(const Context &ctx);

    void LoadPalette(const Palette &palette, int slot);
    void ClearPalette(int slot);
    const auto GetPickerPaletteNum() const { return mPickerPalNum; }
    void SetPickerPaletteNum(unsigned int palNum) { mPickerPalNum = palNum; mRedrawFlag = true; }

    void ResizeMapTexture(int width, int height);
    void Redraw(void) { mRedrawFlag = true; }

private:
    static constexpr int MaxQuads = 20000;
    static constexpr int MaxVertices = 20000 * 4;
    static constexpr int MaxIndices = 20000 * 6;

    struct MapVertex final
    {
        ImVec2 pos;
        ImVec2 uv;
        float palette;
    };

    void DrawTileset(void);
    void DrawTilemap(const Context &ctx);

    void InitializePicker(void);
    void InitializeMap(void);
    void LoadPalette(const void *data, int slot);
    void CreatePaletteTexture(void);
    void LoadTexture(const std::string &fname, Texture &);

    void GenerateTexture(Texture &);
    void GenerateRenderTarget(RenderTarget &target);

    void DeleteTexture(const Texture &);
    void DeleteRenderTarget(const RenderTarget &);
    void SpecifyRenderTargetSize(RenderTarget &, int, int);

    void BatchTile(unsigned short x, unsigned short y, const Tile &tile);
    void BatchBackground(const Tile &tile);
    void FlushRender(void);

    unsigned int mVAO;
    unsigned int mPickerVBO, mPickerEBO;
    unsigned int mMapVBO, mMapEBO;
    unsigned int mPickerShader, mMapShader;
    Texture mPaletteTex, mTilesetTexes[2];
    RenderTarget mPickerTexes[2], mMapTex;

    ImVec2 mMapSize{0, 0};
    MapVertex mMapVertices[MaxVertices];
    unsigned int mMapVertexCount = 0;

    int mPickerPalNum = 0;
    bool mRedrawFlag = true;
};