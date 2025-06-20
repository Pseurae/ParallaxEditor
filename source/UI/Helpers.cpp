#include "UI/Helpers.h"
#include "Global.h"
#include "Core/Snapshot.h"
#include "Utils/FileDialog.h"
#include "Helpers.h"
#include "Popups/Error.h"
#include <filesystem>
#include <fstream>

void CreateNewTilemap(int width, int height)
{
    global.context.New(width, height);
    global.renderer.ResizeMapTexture(width, height);
    action_stack_clear();
}

void TryLoadTilemap(void)
{
    static std::string s;
    if (FileDialog::Open(FileDialog::Mode::Open, {{"Tilemap", "toml"}}, s, s))
    {
        global.context.Load(s);
        global.renderer.ResizeMapTexture(global.context.GetWidth(), global.context.GetHeight());
        global.renderer.Redraw();
        action_stack_clear();
    }
}

void TrySaveTilemap(void)
{
    static std::string p;
    if (FileDialog::Open(FileDialog::Mode::Save, {{ "Tilemap", "toml" }}, p, p))
        global.context.Save(p);
}

std::vector<Tile> LoadBinaryTilemap(const std::string &path)
{
    std::vector<Tile> tiles{};
    std::ifstream fs(path, std::ios::binary);

    while (true)
    {
        unsigned short tileEntry;
        fs.read(reinterpret_cast<char *>(&tileEntry), 2);

        if (fs.eof())
            break;

        tiles.push_back(Tile{
            (tileEntry & 0x400) == 0x400,
            (tileEntry & 0x800) == 0x800,
            (unsigned short)(tileEntry & 0x3FF),
            (unsigned char)((tileEntry >> 12) & 0xF)
        });
    };

    fs.close();
    return tiles;
}

void TryImportTilemap(const std::vector<Tile> &tiles, int width)
{
    global.context.Import(tiles, width, tiles.size() / width);
    global.renderer.ResizeMapTexture(width, tiles.size() / width);
    action_stack_clear();
}

void TryExportTilemap(void)
{
    static std::string p;
    if (FileDialog::Open(FileDialog::Mode::Save, {{ "Tilemap", "bin" }}, p, p))
        global.context.Export(p);
}

void TryOpenProjectFolder(void)
{
    std::string p;
    if (FileDialog::Open(FileDialog::Mode::Folder, {}, p))
        global.context.OpenProjectFolder(p);
}

void TryLoadPrimaryTileset(const std::string &p)
{
    global.context.TilesetPaths()[0] = p;
    if (!global.renderer.LoadPrimaryTileset(p))
        global.popupManager.Open<Popups::Error>("Could not load primary tileset!");
}

void TryLoadSecondaryTileset(const std::string &p)
{
    global.context.TilesetPaths()[1] = p;
    if (!global.renderer.LoadSecondaryTileset(p))
        global.popupManager.Open<Popups::Error>("Could not load secondary tileset!");
}

void UnloadAllPalettes(void)
{
    for (int i = 0; i < 16; ++i)
        global.renderer.ClearPalette(i);
    global.renderer.Redraw();
}

void OpenPalette(unsigned int slot)
{
    static std::string s;

    if (FileDialog::Open(FileDialog::Mode::Open, {{"Palette", "pal"}}, s, s))
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

void OpenPaletteFolder(const std::string &s, int start, int end)
{
    std::filesystem::path paletteFolderPath = s;

    for (int i = start; i < end; ++i)
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

void TryLoadUnderlay(void)
{
    static std::string p;
    if (FileDialog::Open(FileDialog::Mode::Open, {{ "Underlay", "png" }}, p, p))
        global.renderer.LoadUnderlay(p);
}
