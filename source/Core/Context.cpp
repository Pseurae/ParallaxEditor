#include "Core/Context.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "Context.h"
#include "ParallaxEditor.h"
#include <nlohmann/json.hpp>
#include <fex/lexer.h>
#include <fex/parser.h>
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
    for (unsigned int y = 0; y < height * 2; ++y)
    for (unsigned int x = 0; x < width * 2; ++x)
    {
        mTiles[TilePosition{x, y}] = tiles[x + y * (width * 2)];
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
    LoadTilesets();
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

void Context::LoadTilesets(void)
{
    std::string filePath = PathWithRoot("src/data/tilesets/headers.h");
    auto cParser = fex::Parser();
    auto tokens = fex::Lexer().LexFile(filePath);
    auto topLevelObjects = cParser.ParseTopLevelObjects(tokens);

    for (auto it = topLevelObjects.begin(); it != topLevelObjects.end(); it++) 
    {
        std::string structLabel = it->first;
        if (structLabel.empty()) continue;

        Tileset t;
        for (const fex::ArrayValue &v : it->second.values()) 
        {
            if (v.type() == fex::ArrayValue::Type::kValuePair) 
            {
                std::string key = v.pair().first;
                std::string value = v.pair().second->ToString();

                if (key == "tiles") t.tilesPath = GetIncBinFromSymbol("src/data/tilesets/graphics.h", value, "png");
                else if (key == "metatiles") t.metatilesPath = GetIncBinFromSymbol("src/data/tilesets/metatiles.h", value, "bin");
                else if (key == "palettes") t.palettePaths = GetIncBinArrayFromSymbol("src/data/tilesets/graphics.h", value, "pal");
            }
        }

        mTilesets[structLabel] = t;
    }
}

std::string Context::GetIncBinFromSymbol(const std::string &fname, const std::string &sym, const std::string &ext)
{
    std::string path{};
    auto text = LoadTextFile(PathWithRoot(fname));

    std::regex re("\\b" + sym + "\\b\\s*\\[\\]\\s*=\\s*INCBIN_[US][0-9]+\\s*\\(\\s*\"([^\"]*)\"\\s*\\)");
    std::smatch base_match;

    if (std::regex_search(text, base_match, re))
    {
        if (base_match.size() == 2)
        {
            path = base_match[1].str();
            if (!ext.empty())
                path = path.substr(0, path.find_first_of(".")) + "." + ext;
        }
    }

    return path;
}

std::vector<std::string> Context::GetIncBinArrayFromSymbol(const std::string &fname, const std::string &sym, const std::string &ext)
{
    std::vector<std::string> paths{};
    if (sym.empty())
        return paths;

    auto text = LoadTextFile(PathWithRoot(fname));
    std::regex re(sym + "\\s*\\[([^;]*?)\\};");
    std::smatch base_match;

    if (!(std::regex_search(text, base_match, re) && base_match.size() == 2))
        return paths;

    text = base_match[1].str();

    std::regex incbin_re("INCBIN_[SU][0-9]+\\s*\\(\\s*\"(.+)\"\\s*\\)");
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), incbin_re);
    auto words_end = std::sregex_iterator();
 
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
    {
        std::smatch match = *i;
        std::string path = match[1].str();

        if (!ext.empty())
            path = path.substr(0, path.find_first_of(".")) + "." + ext;
        paths.push_back(path);
    }

    return paths;
}