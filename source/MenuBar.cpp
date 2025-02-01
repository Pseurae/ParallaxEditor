#include "MenuBar.h"
#include "FileDialog.h"
#include <imgui.h>
#include "Utils.h"
#include "ActionStack.h"

void main_menu_bar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Tilemap", "Ctrl+O"))
                open_tilemap();

            if (ImGui::MenuItem("Save Tilemap", "Ctrl+S"))
                save_tilemap();
            
            if (ImGui::MenuItem("Save As Tilemap", "Ctrl+Shift+S"))
                save_as_tilemap();

            if (ImGui::MenuItem("Open Primary Tileset", "Ctrl+Shift+1"))
                open_primary_tileset();

            if (ImGui::MenuItem("Open Secondary Tileset", "Ctrl+Shift+2"))
                open_secondary_tileset();

            if (ImGui::MenuItem("Open Palettes", "Ctrl+Shift+O"))
                open_palettes();

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