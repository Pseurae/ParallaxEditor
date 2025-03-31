#pragma once

#include "Core/Popup.h"
#include "Utils/Tile.h"

namespace Popups
{
class ImportTilemap final : public Popup
{
public:
    ImportTilemap() : Popup("Import Tilemap")
    {}

    void DrawContent(void) override;
private:
    std::string mPath;
    int mWidth = 1;
    std::vector<Tile> mTiles{};
};
}