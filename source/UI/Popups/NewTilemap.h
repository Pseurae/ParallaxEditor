#pragma once

#include "Core/Popup.h"

namespace Popups
{
class NewTilemap final : public Popup
{
public:
    NewTilemap() : Popup("New Tilemap")
    {}
    void DrawContent() override;
private:
    int mWidth = 32, mHeight = 32;
};
}