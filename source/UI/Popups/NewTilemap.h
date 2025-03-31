#pragma once

#include "Core/Popup.h"

namespace Popups
{
class NewTilemap final : public Popup
{
public:
    NewTilemap() : Popup("New Tilemap")
    {}
    void DrawContent(void) override;
private:
    int mWidth = 32, mHeight = 32;
    int mDefaultTile = 0;
};
}