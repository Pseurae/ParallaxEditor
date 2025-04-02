#include "Core/Context.h"
#include <filesystem>
#include <iostream>
#include "Context.h"
#include "ParallaxEditor.h"
#include <toml++/toml.hpp>

void Context::New(int width, int height)
{
    mWidth = width;
    mHeight = height;
    mTiles.clear();
    mLoaded = true;
}

void Context::Load(const std::string &fname)
{
    auto tbl = toml::parse_file(fname);

    mWidth = tbl["width"].value_or(32);
    mHeight = tbl["height"].value_or(32);

    auto defaultTile = tbl["default_tile"];

    auto readTile = +[](const toml::table &tbl) {
        Tile tile;
        tile.id = tbl["id"].value_or(0);
        tile.palette = tbl["palette"].value_or(0);
        tile.xflip = tbl["xflip"].value_or(0);
        tile.yflip = tbl["yflip"].value_or(0);

        return tile;
    };

    if (toml::table *defaultTileTbl = defaultTile.as_table())
        mDefaultTile = readTile(*defaultTileTbl);

    auto tiles = tbl["tiles"];

    if (toml::array *tileArray = tiles.as_array())
    {
        mTiles.clear();
        tileArray->for_each([readTile, this](auto &&i) {
            if (auto tbl = i.as_table())
            {
                Tile tile = readTile(*tbl);
                auto x = (*tbl)["x"].template value<unsigned int>();
                auto y = (*tbl)["y"].template value<unsigned int>();

                if (x && y)
                    mTiles[TilePosition{x.value(), y.value()}] = tile;
            }
        });
    }

    mLoaded = true;
}

void Context::Save(const std::string &path)
{
    toml::table tbl{
        { "width", mWidth },
        { "height", mHeight },
        { "default_tile", toml::table{
                { "id", mDefaultTile.id },
                { "xflip", mDefaultTile.xflip },
                { "yflip", mDefaultTile.yflip }
            }
        },
    };

    toml::array tilesArray{};
    for (const auto &[pos, tile] : mTiles)
    {
        tilesArray.push_back(toml::table{
            { "id", tile.id },
            { "xflip", tile.xflip },
            { "yflip", tile.yflip },
            { "palette", tile.palette },
            { "x", pos.x },
            { "y", pos.y },
        });
    }
    tbl.emplace("tiles", tilesArray);

    std::ofstream fs(path);
    fs << tbl;
    fs.close();
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
