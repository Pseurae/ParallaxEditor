#include "UI/Popups/NewTilemap.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

namespace Popups
{
void NewTilemap::DrawContent()
{
    ImGui::InputInt("Width", &mWidth);
    ImGui::InputInt("Height", &mHeight);

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