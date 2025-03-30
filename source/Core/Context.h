#pragma once

#include <string>
#include "Utils/Tilemap.h"
#include "Utils/Palette.h"

class Context final
{
public:
    Context(const std::string &fname);
    Context(unsigned short width, unsigned short height);

    void TrySave(void);
    void SaveAs(const std::string &path);

    void Import(const std::string &path);
    void Export(const std::string &path);

    const std::string GetName() const;
    inline bool &IsDirty() { return mDirty; }

    Tilemap &GetTilemap(void) { return mTilemap; }

private:
    void Load(const std::string &fname);

    std::string mPath;
    Palette mPalettes[16];
    Tilemap mTilemap{0, 0};
    std::string mTilesetPaths[2];
    bool mIs8BPP;
    bool mDirty = false;
};