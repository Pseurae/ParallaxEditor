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
    mLoaded = true;
}

void Context::Load(const std::string &fname)
{
    mTiles.clear();

    auto tbl = toml::parse_file(fname);

    auto width = tbl["width"].value<int>();
    auto height = tbl["height"].value<int>();
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

    auto palettes = tbl["palettes"];
    auto tilesets = tbl["tilesets"];

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

    toml::array palettesArray;
    for (int i = 0; i < mPalettePaths.size(); ++i)
    {
        const auto &path = mPalettePaths[i];
        if (path.empty()) continue;

        palettesArray.push_back(toml::table{
            {"slot", i},
            {"path", path}
        });
    }

    tbl.emplace("palettes", palettesArray);

    toml::array tilesetArray;
    for (int i = 0; i < mTilesetPaths.size(); ++i)
    {
        const auto &path = mTilesetPaths[i];
        if (path.empty()) continue;

        tilesetArray.push_back(toml::table{
            { "slot", i },
            { "path", path }
        });
    }

    tbl.emplace("tilesets", tilesetArray);

    mPath = path;
    std::ofstream f(path);
    f << tbl;
    f.close();
}

void Context::Import(const std::string &path)
{}

void Context::Export(const std::string &path)
{}

void Context::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

const std::string Context::GetName() const
{
    return mPath.empty() ? "Untitled" : std::filesystem::path(mPath).filename().string();
}
