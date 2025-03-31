#include "UI/Popups/Prompt.h"
#include <imgui.h>

namespace Popups
{
void Prompt::DrawContent(void)
{
    ImGui::Text("%s", mMessage.c_str());

    if (ImGui::Button("Yes"))
    {
        if (mYesAction) mYesAction();
        Close();
    }

    ImGui::SameLine();

    if (ImGui::Button("No"))
    {
        if (mNoAction) mNoAction();
        Close();
    }
}
}