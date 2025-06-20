#include "Core/Context.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "Context.h"
#include "ParallaxEditor.h"

void Context::New(int width, int height)
{
    mWidth = width;
    mHeight = height;
    mTiles.clear();
    mLoaded = true;
}

void Context::Import(const std::vector<Tile> &tiles, int width, int height)
{
    mWidth = width;
    mHeight = height;

    mTiles.clear();
    for (unsigned int y = 0; y < height; ++y)
    for (unsigned int x = 0; x < width; ++x)
    {
        mTiles[TilePosition{x, y}] = tiles[x + y * width];
    }

    mLoaded = true;
}

static inline unsigned short ConvertTileToGBA(const Tile &tile)
{
    return (tile.id & 0x3FF) |
        (tile.xflip ? 0x400 : 0) |
        (tile.yflip ? 0x800 : 0) |
        (tile.palette & 0xF);
}

struct GbaTile
{
    unsigned short tile:10;
    unsigned short xflip:1;
    unsigned short yflip:1;
    unsigned short bank:4;
};

void Context::Export(const std::string &path)
{
    std::ofstream fs(path);
    for (unsigned int y = 0; y < mHeight; ++y)
    for (unsigned int x = 0; x < mWidth; ++x)
    {
        const auto &tile = mTiles.contains({ x, y }) ? mTiles.at({ x, y }) : mDefaultTile;
        // unsigned short bytes = ConvertTileToGBA(tile);
        GbaTile gbaTile{tile.id, tile.xflip, tile.yflip, tile.palette};
        fs.write(reinterpret_cast<char *>(&gbaTile), 2);
    }
    fs.close();
}

void Context::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

const std::string Context::GetName() const
{
    return mPath.empty() ? "Untitled" : std::filesystem::path(mPath).filename().string();
}

void Context::AddTile(const TilePosition &pos, const Tile &tile)
{
    mTiles[pos] = tile;
}

void Context::OpenProjectFolder(const std::string &fname)
{
    mPrimaryTilesets.clear();
    mSecondaryTilesets.clear();

    std::filesystem::path tilesetFolderPath = std::filesystem::path(fname) / "data" / "tilesets";

    for (const auto &entry : std::filesystem::directory_iterator(tilesetFolderPath / "primary"))
    {
        if (entry.is_directory())
            mPrimaryTilesets.push_back(entry.path().string());
    }

    for (const auto &entry : std::filesystem::directory_iterator(tilesetFolderPath / "secondary"))
    {
        if (entry.is_directory())
            mSecondaryTilesets.push_back(entry.path().string());
    }

    std::sort(mPrimaryTilesets.begin(), mPrimaryTilesets.end());
    std::sort(mSecondaryTilesets.begin(), mSecondaryTilesets.end());

    mProjectLoaded = true;
}