#pragma once

#include <string>
#include <unordered_map>
#include <array>
#include <vector>
#include "Utils/Palette.h"
#include "Utils/Tile.h"

class Context final
{
public:
    Context() = default;

    void New(int width, int height);

    void Import(const std::vector<Tile> &tiles, int width, int height);
    void Export(const std::string &path);

    void Resize(int width, int height);

    const std::string GetName() const;
    inline bool &IsDirty() { return mDirty; }

    const std::string &GetPath() const { return mPath; }
    const auto &GetTiles(void) const { return mTiles; }
    void SetTiles(const std::unordered_map<TilePosition, Tile> &tiles) { mTiles = tiles; }

    void OpenProjectFolder(const std::string &fname);
    
    auto &PalettePaths(void) { return mPalettePaths; }
    auto &TilesetPaths(void) { return mTilesetPaths; }

    bool IsLoaded(void) { return mLoaded; }

    const unsigned short GetWidth() const { return mWidth; }
    const unsigned short GetHeight() const { return mHeight; }

    const Tile GetDefaultTile() const { return mDefaultTile; }

    void AddTile(const TilePosition &pos, const Tile &tile);

    const auto &GetBlockData() const { return mBlockData; }
    void LoadBlockData(const std::vector<unsigned short> &blockData);

    void LoadPrimaryMetatiles(const std::vector<Tile> &tiles);
    void LoadSecondaryMetatiles(const std::vector<Tile> &tiles);

    const auto &GetPrimaryMetatiles(void) const { return mPrimaryMetatiles; }
    const auto &GetSecondaryMetatiles(void) const { return mSecondaryMetatiles; }

private:
    std::string mPath;

    unsigned short mWidth = 0, mHeight = 0;
    Tile mDefaultTile;
    std::unordered_map<TilePosition, Tile> mTiles;

    std::array<std::string, 16> mPalettePaths;
    std::array<std::string, 2> mTilesetPaths;

    std::vector<unsigned short> mBlockData;
    std::array<Tile, 24576> mPrimaryMetatiles{0}, mSecondaryMetatiles{0};

    bool mIs8BPP = false;
    bool mDirty = false;
    bool mLoaded = false, mProjectLoaded = false;
};