#include "UI/Helpers.h"
#include "Global.h"
#include "Utils/FileDialog.h"
#include "Helpers.h"
#include <filesystem>

void CreateNewTilemap(int width, int height)
{
    global.context.New(width, height);
    global.renderer.ResizeMapTexture(width, height);
}

void TryLoadTilemap(void)
{
    std::string s;
    if (FileDialog::Open(FileDialog::Mode::Open, {{"Tilemap", "toml"}}, s))
    {
        global.context.Load(s);
        global.renderer.ResizeMapTexture(global.context.GetWidth(), global.context.GetHeight());
        global.renderer.Redraw();
    }
}

void TrySaveTilemap(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Save, {{ "Tilemap", "toml" }}, p))
        global.context.Save(p);
}

void TryExportTilemap(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Save, {{ "Tilemap", "bin" }}, p))
        global.context.Export(p);
}

void TryLoadPrimaryTileset(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Open, {{ "Tileset", "png" }}, p))
    {
        global.context.TilesetPaths()[0] = p;
        global.renderer.LoadPrimaryTileset(p);
    }
}

void TryLoadSecondaryTileset(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Open, {{ "Tileset", "png" }}, p))
    {
        global.context.TilesetPaths()[1] = p;
        global.renderer.LoadSecondaryTileset(p);
    }
}

void UnloadAllPalettes(void)
{
    for (int i = 0; i < 16; ++i)
        global.renderer.ClearPalette(i);
    global.renderer.Redraw();
}

void OpenPalette(unsigned int slot)
{
    std::string s;

    if (FileDialog::Open(FileDialog::Mode::Open, {{"Palette", "pal"}}, s))
    {
        global.context.PalettePaths()[slot] = s;
        global.renderer.LoadPalette(Palette(s), slot);

        if (global.renderer.GetPickerPaletteNum() == slot)
            global.renderer.Redraw();
    }
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

void OpenPaletteFolder(void)
{
    std::string s;
    if (FileDialog::Open(FileDialog::Mode::Folder, {}, s))
    {
        std::filesystem::path paletteFolderPath = s;

        for (int i = 0; i < 16; ++i)
        {
            auto palPath = paletteFolderPath / gPaletteFileNames[i];
    
            if (std::filesystem::exists(palPath))
            {
                global.context.PalettePaths()[i] = palPath.string();
                global.renderer.LoadPalette(Palette(palPath.string()), i);
            }
        }

        global.renderer.Redraw();
    }
}