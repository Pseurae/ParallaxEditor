#include "UI/Popups/Error.h"
#include <imgui.h>

namespace Popups
{
void Error::DrawContent()
{
    ImGui::Text("%s", mMessage.c_str());

    if (ImGui::Button("Close"))
        Close();
}
}