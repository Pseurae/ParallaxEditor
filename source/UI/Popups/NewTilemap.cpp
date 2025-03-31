#include "UI/Popups/NewTilemap.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

namespace Popups
{
void NewTilemap::DrawContent()
{
    if (ImGui::InputInt("Width", &mWidth))
        mWidth = std::max(0, mWidth);

    if (ImGui::InputInt("Height", &mHeight))
        mWidth = std::max(0, mHeight);

    if (ImGui::Button("Ok"))
    {
        CreateNewTilemap(mWidth, mHeight);
        Close();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        Close();
    }
}
}