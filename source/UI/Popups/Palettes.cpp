#include "UI/Popups/Palettes.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"
#include "Utils/FileDialog.h"

namespace Popups
{
void Palettes::DrawContent()
{
    auto palettePaths = global.context.PalettePaths();

    for (int i = 0; i < 16; ++i)
    {
        ImGui::Text("%02d", i);
        ImGui::SameLine();

        ImGui::TextWrapped("%s", (palettePaths[i].empty() ? "Empty" : palettePaths[i].c_str()));

        ImGui::SameLine();

        ImGui::PushID(i);
        if (ImGui::Button("Open"))
        {
            std::string s;

            if (FileDialog::Open(FileDialog::Mode::Open, {{"Palette", "pal"}}, s))
            {
                global.context.PalettePaths()[i] = s;
                global.renderer.LoadPalette(Palette(s), i);
            }
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Close"))
        Close();
}
}