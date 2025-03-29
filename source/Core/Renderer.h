#pragma once

#include <string>
#include "Utils/Palette.h"

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

    void DrawTileset(void);
    void DrawTilemap(void);
    void Draw(void);

    void LoadPalette(const Palette &palette, int slot);
    const auto GetPickerPaletteNum() const { return mPickerPalNum; }
    void SetPickerPaletteNum(unsigned int palNum) { mPickerPalNum = palNum; mRedrawFlag = true; }

private:
    void LoadPalette(const void *data, int slot);
    void CreatePaletteTexture(void);
    void LoadTexture(const std::string &fname, Texture &);

    void GenerateTexture(Texture &);
    void GenerateRenderTarget(RenderTarget &target);

    void DeleteTexture(const Texture &);
    void DeleteRenderTarget(const RenderTarget &);
    void SpecifyRenderTargetSize(RenderTarget &, int, int);

    unsigned int mVAO;
    unsigned int mPickerVBO, mPickerEBO;
    unsigned int mPickerShader, mMapShader;
    Texture mPaletteTex, mTilesetTexes[2];
    RenderTarget mPickerTexes[2], mMapTex;

    int mPickerPalNum = 0;

    bool mRedrawFlag = true;
};