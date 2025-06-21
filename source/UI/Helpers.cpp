#include "UI/Helpers.h"
#include "Global.h"
#include "Core/Snapshot.h"
#include "Utils/FileDialog.h"
#include "Helpers.h"
#include "Popups/Error.h"
#include <filesystem>
#include <fstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

void CreateNewTilemap(int width, int height)
{
    global.context.New(width, height);
    global.renderer.ResizeMapTexture(width, height);
    action_stack_clear();
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

template<class T>
static inline void swap_val(T *v1, T *v2)
{
    T temp = *v1;
    *v1 = *v2;
    *v2 = temp;
}

ImRect GetSelectionRectFromDrag(ImVec2 start, ImVec2 end, const ImVec2 &tileSize)
{
    if (start.x > end.x) swap_val(&start.x, &end.x);
    if (start.y > end.y) swap_val(&start.y, &end.y);

    start /= tileSize;
    end /= tileSize;

    start.x = std::floorf(start.x);
    start.y = std::floorf(start.y);

    end.x = std::ceilf(end.x);
    end.y = std::ceilf(end.y);

    return ImRect(start, end);
}

std::vector<unsigned short> LoadBinaryBlockData(const std::string &path)
{
    std::vector<unsigned short> tiles{};
    std::ifstream fs(path, std::ios::binary);

    while (true)
    {
        unsigned short tileEntry;
        fs.read(reinterpret_cast<char *>(&tileEntry), 2);

        if (fs.eof())
            break;

        tiles.push_back(tileEntry);
    };

    fs.close();
    return tiles;
}

void TryOpenPrimaryMetatiles(void)
{
    auto primaryTiles = LoadBinaryTilemap("testing/general_metatiles.bin");
    global.context.LoadPrimaryMetatiles(primaryTiles);
    auto secondaryTiles = LoadBinaryTilemap("testing/petalburg_metatiles.bin");
    global.context.LoadSecondaryMetatiles(secondaryTiles);

    global.renderer.LoadPrimaryTileset("testing/general_tiles.png");
    global.renderer.LoadSecondaryTileset("testing/petalburg_tiles.png");

    OpenPaletteFolder("testing/palettes", 0, 13);

    global.context.New(70, 50);
    global.renderer.ResizeMapTexture(70, 50);
    global.context.LoadBlockData(LoadBinaryBlockData("testing/map.bin"));
}