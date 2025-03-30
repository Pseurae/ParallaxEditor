#include <imgui.h>
#include "Global.h"
#include "Popups/NewTilemap.h"

void main_menu_bar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Files"))
        {
            if (ImGui::MenuItem("New Tilemap"))
                global.popupManager.Open<Popups::NewTilemap>();

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}