#include "UI/OptionsPane.h"
#include <filesystem>
#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"

void OptionsPane(void)
{
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Load Underlay"))
        TryLoadUnderlay();
    if (ImGui::Button("Remove Underlay"))
        global.renderer.LoadEmptyUnderlay();

    ImGui::SliderFloat("Opacity", &global.transparency, 0.0, 1.0);

    const auto &primaryTilesets = global.context.GetPrimaryTilesets();
    static int primaryTilesetIdx = 0;
    std::string primaryName = std::filesystem::path(primaryTilesets[primaryTilesetIdx]).filename().string();

    if (ImGui::BeginCombo("Primary", primaryName.c_str()))
    {
        for (int n = 0; n < primaryTilesets.size(); ++n)
        {
            const bool is_selected = (primaryTilesetIdx == n);
            std::filesystem::path tilesetFolder = std::filesystem::path(primaryTilesets[n]);
            std::string tilesetName = tilesetFolder.filename().string();
            std::filesystem::path tilesetPath =  tilesetFolder / "tiles.png";

            if (ImGui::Selectable(tilesetName.c_str(), is_selected))
            {
                primaryTilesetIdx = n;
                TryLoadPrimaryTileset(tilesetPath.string());
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    const auto &secondaryTilesets = global.context.GetSecondaryTilesets();
    static int secondaryTilesetIdx = 0;
    std::string secondaryName = std::filesystem::path(secondaryTilesets[secondaryTilesetIdx]).filename().string();

    if (ImGui::BeginCombo("Secondary", secondaryName.c_str()))
    {
        static ImGuiTextFilter secondaryFilter;
        if (ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
            secondaryFilter.Clear();
        }

        secondaryFilter.Draw("##SecFilter", -FLT_MIN);

        for (int n = 0; n < secondaryTilesets.size(); ++n)
        {
            const bool is_selected = (secondaryTilesetIdx == n);
            std::filesystem::path tilesetFolder = std::filesystem::path(secondaryTilesets[n]);
            std::string tilesetName = tilesetFolder.filename().string();
            std::filesystem::path tilesetPath =  tilesetFolder / "tiles.png";
            std::filesystem::path tilesetPalettes =  tilesetFolder / "palettes";

            if (secondaryFilter.PassFilter(tilesetName.c_str()))
            {
                if (ImGui::Selectable(tilesetName.c_str(), is_selected))
                {
                    secondaryTilesetIdx = n;
                    TryLoadSecondaryTileset(tilesetPath.string());
                    OpenPaletteFolder(tilesetPalettes.string());
                }
            }
        }
        ImGui::EndCombo();
    }

    ImGui::End();
}