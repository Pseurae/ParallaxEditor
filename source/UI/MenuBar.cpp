#include <imgui.h>
#include "Global.h"
#include "UI/Helpers.h"
#include "UI/Popups/ImportTilemap.h"
#include "UI/Popups/NewTilemap.h"
#include "UI/Popups/Palettes.h"
#include "Utils/FileDialog.h"

void MainMenuBar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Tilemap"))
        {
            if (ImGui::MenuItem("New Tilemap"))
                global.popupManager.Open<Popups::NewTilemap>();

            if (ImGui::MenuItem("Open Tilemap"))
                TryLoadTilemap();

            if (ImGui::MenuItem("Save Tilemap", nullptr, nullptr, global.context.IsLoaded()))
                TrySaveTilemap();

            ImGui::Separator();

            if (ImGui::MenuItem("Import Tilemap", nullptr, nullptr))
                global.popupManager.Open<Popups::ImportTilemap>();

            if (ImGui::MenuItem("Export Tilemap", nullptr, nullptr, global.context.IsLoaded()))
                TryExportTilemap();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tileset"))
        {
            if (ImGui::MenuItem("Load Primary Tileset", nullptr, nullptr, global.context.IsLoaded()))
                TryLoadPrimaryTileset();

            if (ImGui::MenuItem("Load Secondary Tileset", nullptr, nullptr, global.context.IsLoaded()))
                TryLoadSecondaryTileset();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Palettes"))
        {
            if (ImGui::MenuItem("Change Palettes", nullptr, nullptr, global.context.IsLoaded()))
                global.popupManager.Open<Popups::Palettes>();
            
            if (ImGui::MenuItem("Unload All Palettes", nullptr, nullptr, global.context.IsLoaded()))
                UnloadAllPalettes();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Resize", nullptr, nullptr, global.context.IsLoaded()))
                ;
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}