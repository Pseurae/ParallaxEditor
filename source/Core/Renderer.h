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
        int width, height;
        unsigned int id;
    };

    struct RenderTarget
    {
        unsigned fbo;
        Texture tex;
    };

    void Initialize(void);
    void Shutdown(void);

    const Texture &GetPickerTex(void) const { return mPickerTex.tex; }
    const Texture &GetMapTex(void) const { return mMapTex.tex; }
    const Texture &GetUnderlayTex(void) const { return mUnderlayTex; }

    void LoadPrimaryTileset(const std::string &fname);
    void LoadSecondaryTileset(const std::string &fname);
    void LoadUnderlay(const std::string &fname);
    void Draw(const Context &ctx);

    void LoadPalette(const Palette &palette, int slot);
    void ClearPalette(int slot);
    const auto GetPickerPaletteNum(void) const { return mPickerPalNum; }
    void SetPickerPaletteNum(unsigned char palNum) { mPickerPalNum = palNum; mRedrawFlag = true; }

    void ResizeMapTexture(int width, int height);
    void Redraw(void) { mRedrawFlag = true; }

    void LoadEmptyUnderlay(void);

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
    void FlushTilemap(void);
    void DrawTilemap(const Context &ctx);

    void InitializePicker(void);
    void InitializeMap(void);
    void LoadPalette(const void *data, int slot);
    void CreatePaletteTexture(void);
    void CreateUnderlayTexture(unsigned int width, unsigned int height, const unsigned char *data);
    void CreateTexture(unsigned int width, unsigned int height, Texture &);

    void GenerateTexture(Texture &);
    void GenerateRenderTarget(RenderTarget &target);

    void DeleteTexture(const Texture &);
    void DeleteRenderTarget(const RenderTarget &);
    void SpecifyRenderTargetSize(RenderTarget &, int, int);

    void BatchTile(unsigned short x, unsigned short y, const Tile &tile);
    void BatchBackground(const Tile &tile);

    unsigned int mVAO;
    unsigned int mPickerVBO, mPickerEBO;
    unsigned int mMapVBO, mMapEBO;
    unsigned int mPickerShader, mMapShader;
    Texture mPaletteTex, mTilesetTex, mUnderlayTex;
    RenderTarget mPickerTex, mMapTex;

    MapVertex mMapVertices[MaxVertices];
    unsigned int mMapQuadCount = 0;

    unsigned char mPickerPalNum = 0;
    bool mRedrawFlag = true;
};