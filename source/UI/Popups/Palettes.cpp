#include "UI/Popups/Palettes.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"
#include "Utils/FileDialog.h"
#include <filesystem>

namespace Popups
{
void Palettes::DrawContent()
{
    auto palettePaths = global.context.PalettePaths();

    for (int i = 0; i < 16; ++i)
    {
        ImGui::Text("%2d", i);
        ImGui::SameLine();

        ImGui::TextWrapped("%s", (palettePaths[i].empty() ? "Empty" : std::filesystem::path(palettePaths[i]).filename().string().c_str()));
        ImGui::SameLine(350.0f);

        ImGui::PushID(i);
        if (ImGui::Button("Open"))
            OpenPalette(i);
        ImGui::PopID();
    }

    if (ImGui::Button("Close"))
        Close();
    
    ImGui::SameLine();

    if (ImGui::Button("Open from Folder"))
        OpenPaletteFolder();
}
}