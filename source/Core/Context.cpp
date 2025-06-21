#include "Core/Context.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "Context.h"
#include "ParallaxEditor.h"
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

void Context::New(int width, int height)
{
    mWidth = width;
    mHeight = height;
    mTiles.clear();
    mMapLoaded = true;
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

    mMapLoaded = true;
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
    for (unsigned int y = 0; y < mHeight * 2; ++y)
    for (unsigned int x = 0; x < mWidth * 2; ++x)
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

void Context::AddTile(const TilePosition &pos, const Tile &tile)
{
    mTiles[pos] = tile;
}

void Context::OpenProjectFolder(const std::string &fname)
{
    mProjectPath = fname;
    mProjectLoaded = true;

    LoadLayouts();
    LoadMaps();
    LinkMapsToLayouts();
}

void Context::LoadBlockData(const std::vector<unsigned short> &blockData)
{
    mBlockData = blockData;
}

void Context::LoadPrimaryMetatiles(const std::vector<Tile> &tiles)
{
    std::copy(tiles.begin(), tiles.end(), mPrimaryMetatiles.data());
}

void Context::LoadSecondaryMetatiles(const std::vector<Tile> &tiles)
{
    std::copy(tiles.begin(), tiles.end(), mSecondaryMetatiles.data());
}

std::string Context::PathWithRoot(const std::string &path) {
    if (mProjectPath.empty()) return path;
    if (path.starts_with(mProjectPath)) return path;
    return mProjectPath + "/" + path;
}

std::string Context::LoadTextFile(const std::string &fname)
{
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return(contents.str());
    }

    return "";
}

void Context::LoadLayouts(void)
{
    std::ifstream f(PathWithRoot("data/layouts/layouts.json"));
    auto j = json::parse(f);
    Layout l;

    const auto &layouts = j.at("layouts");
    for (const auto &v : layouts)
    {
        const auto &id = v.at("id").get<std::string>();
        v.at("width").get_to(l.width);
        v.at("height").get_to(l.height);
        v.at("primary_tileset").get_to(l.primaryTileset);
        v.at("secondary_tileset").get_to(l.secondaryTileset);
        v.at("blockdata_filepath").get_to(l.blockDataPath);

        mLayouts.insert({ id, l });
    }
}

void Context::LoadMaps(void)
{
    std::ifstream f(PathWithRoot("data/maps/map_groups.json"));
    auto j = json::parse(f);

    for (const auto &[k, v] : j.items())
    {
        if (k == "group_order") mMapGroupOrders = v.template get<std::vector<std::string>>();
        else mGroupedMaps[k] = v.template get<std::vector<std::string>>();
    }
}

void Context::LinkMapsToLayouts(void)
{
    for (const auto &groupName : mMapGroupOrders)
    for (const auto &mapName : mGroupedMaps[groupName])
    {
        std::ifstream f(PathWithRoot("data/maps/" + mapName + "/map.json"));
        auto j = json::parse(f);

        const auto layoutName = j.at("layout").get<std::string>();

        if (!layoutName.empty())
            mMapToLayoutId[mapName] = layoutName;
    }
}