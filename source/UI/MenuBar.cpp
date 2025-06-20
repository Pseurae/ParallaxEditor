#include "UI/Helpers.h"
#include <imgui.h>
#include "Global.h"
#include "UI/Popups/ImportTilemap.h"
#include "UI/Popups/NewTilemap.h"
#include "UI/Popups/Palettes.h"
#include "Utils/FileDialog.h"
#include "Core/Snapshot.h"

void MainMenuBar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Tilemap"))
        {
            if (ImGui::MenuItem("New Tilemap"))
                global.popupManager.Open<Popups::NewTilemap>();

            if (ImGui::MenuItem("Open Tilemap", nullptr, nullptr))
                global.popupManager.Open<Popups::ImportTilemap>();

            if (ImGui::MenuItem("Save Tilemap", nullptr, nullptr, global.context.IsLoaded()))
                TryExportTilemap();

            if (ImGui::MenuItem("Open Project", nullptr, nullptr))
                TryOpenProjectFolder();

            if (ImGui::MenuItem("Open Metatiles"))
                TryOpenPrimaryMetatiles();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, action_stack_can_undo()))
                action_stack_do_undo();

            if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, action_stack_can_redo()))
                action_stack_do_redo();
                
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}