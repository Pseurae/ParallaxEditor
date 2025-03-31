#pragma once

#include <string>
#include <unordered_map>
#include <array>
#include "Utils/Palette.h"
#include "Utils/Tile.h"

class Context final
{
public:
    Context() = default;

    void New(int width, int height);
    void Load(const std::string &fname);
    void Save(const std::string &path);

    void Import(const std::vector<Tile> &tiles, int width, int height);
    void Export(const std::string &path);

    void Resize(int width, int height);

    const std::string GetName() const;
    inline bool &IsDirty() { return mDirty; }

    const std::string &GetPath() const { return mPath; }
    const auto &GetTiles(void) const { return mTiles; }

    auto &PalettePaths(void) { return mPalettePaths; }
    auto &TilesetPaths(void) { return mTilesetPaths; }

    bool IsLoaded(void) { return mLoaded; }

    const unsigned short GetWidth() const { return mWidth; }
    const unsigned short GetHeight() const { return mHeight; }

    const Tile GetDefaultTile() const { return mDefaultTile; }

    void AddTile(const TilePosition &pos, const Tile &tile);

private:
    std::string mPath;

    unsigned short mWidth, mHeight;
    Tile mDefaultTile;
    std::unordered_map<TilePosition, Tile> mTiles;

    std::array<std::string, 16> mPalettePaths;
    std::array<std::string, 2> mTilesetPaths;

    bool mIs8BPP = false;
    bool mDirty = false;
    bool mLoaded = false;
};