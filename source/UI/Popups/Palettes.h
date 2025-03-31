#pragma once

#include "Core/Popup.h"

namespace Popups
{
class Palettes final : public Popup
{
public:
    Palettes() : Popup("Palettes")
    {}

    void DrawContent(void) override;
private:
};
}