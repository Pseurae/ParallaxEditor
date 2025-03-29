#include <imgui.h>
#include "Global.h"

void main_menu_bar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Files"))
        {
            if (ImGui::MenuItem("New Tilemap"))
                global.contexts.push_back(std::make_unique<Context>(12, 12));

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}