#include "UI/TilemapPane.h"
#include <imgui.h>
#include "Global.h"
#include <iostream>

void TilemapPane(void)
{
    if (ImGui::BeginChild("Tilemap", ImVec2(0.0f, 0.0f), 0, ImGuiWindowFlags_HorizontalScrollbar))
    {
        auto &tex = global.renderer.GetMapTex();
        ImGui::Image(tex.id, ImVec2(tex.width, tex.height));
        ImGui::EndChild();
    }
}