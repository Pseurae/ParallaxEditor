#include "UI/Helpers.h"
#include "Global.h"
#include "Utils/FileDialog.h"
#include "Helpers.h"

void CreateNewTilemap(int width, int height)
{
    global.context.New(width, height);
}

void TryLoadTilemap(void)
{
    std::string s;
    if (FileDialog::Open(FileDialog::Mode::Open, {{"Tilemap", "toml"}}, s))
        global.context.Load(s);
}

void TrySaveTilemap(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Save, {{ "Tilemap", "toml" }}, p))
    {
        global.context.Save(p);
    }
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
}
