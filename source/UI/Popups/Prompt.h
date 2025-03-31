#pragma once

#include "Core/Popup.h"
#include <functional>

namespace Popups
{
class Prompt final : public Popup
{
public:
    using Action = std::function<void(void)>;
    Prompt(const std::string &message, Action yesAction, Action noAction) : 
        Popup("Prompt"), mMessage(message), mYesAction(yesAction), mNoAction(noAction)
    {}

    void DrawContent(void) override;
private:
    std::string mMessage;
    Action mYesAction, mNoAction;
};
}