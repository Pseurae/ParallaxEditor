#pragma once

#include "Core/Popup.h"

namespace Popups
{
class Error final : public Popup
{
public:
    Error(const std::string &message) : Popup("Error"), mMessage(message)
    {}

    void DrawContent(void) override;

private:
    std::string mMessage;
};
}