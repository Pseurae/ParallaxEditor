#pragma once

#include <string>
#include <unordered_map>
#include <array>
#include <vector>
#include "Utils/Palette.h"
#include "Utils/Tile.h"

struct Layout
{
    int width, height;
    std::string primaryTileset, secondaryTileset;
    std::string blockDataPath;
};

class Context final
{
public:
    Context() = default;

    void New(int width, int height);

    void Import(const std::vector<Tile> &tiles, int width, int height);
    void Export(const std::string &path);

    void Resize(int width, int height);

    inline bool &IsDirty() { return mDirty; }

    const auto &GetTiles(void) const { return mTiles; }
    void SetTiles(const std::unordered_map<TilePosition, Tile> &tiles) { mTiles = tiles; }

    const std::string &GetProjectPath() const { return mProjectPath; }
    void OpenProjectFolder(const std::string &fname);
    bool IsProjectLoaded(void) { return mProjectLoaded; }
    
    auto &PalettePaths(void) { return mPalettePaths; }
    auto &TilesetPaths(void) { return mTilesetPaths; }

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

    bool IsMapLoaded(void) const { return mMapLoaded; }
    const auto &GetMapOrderLabels(void) const { return mMapGroupOrders; }
    const auto &GetGroupedMapLabels(void) const { return mGroupedMaps; }

    std::string PathWithRoot(const std::string &fname);
    const auto &GetMapLayout(const std::string &mapName) { return mLayouts[mMapToLayoutId[mapName]]; }

private:
    std::string LoadTextFile(const std::string &fname);

    void LoadLayouts(void);
    void LoadMaps(void);
    void LinkMapsToLayouts(void);

    std::string mProjectPath;

    unsigned short mWidth = 0, mHeight = 0;
    Tile mDefaultTile;
    std::unordered_map<TilePosition, Tile> mTiles;

    std::array<std::string, 16> mPalettePaths;
    std::array<std::string, 2> mTilesetPaths;

    std::vector<unsigned short> mBlockData;
    std::array<Tile, 24576> mPrimaryMetatiles{0}, mSecondaryMetatiles{0};

    std::unordered_map<std::string, Layout> mLayouts;
    std::vector<std::string> mMapGroupOrders;
    std::unordered_map<std::string, std::vector<std::string>> mGroupedMaps;

    std::unordered_map<std::string, std::string> mMapToLayoutId;

    bool mIs8BPP = false;
    bool mDirty = false;
    bool mProjectLoaded = false, mMapLoaded = false;
};