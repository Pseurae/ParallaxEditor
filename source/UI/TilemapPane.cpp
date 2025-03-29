#include "UI/TilemapPane.h"
#include <imgui.h>

void TilemapPane(void)
{
    if (ImGui::BeginChild("Tilemap", ImVec2(0.0f, 0.0f), 0, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("Test");
        ImGui::EndChild();
    }
}