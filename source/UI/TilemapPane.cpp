#include "UI/TilemapPane.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "Global.h"
#include <iostream>

static void TilemapWindow(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    bool mouseDown = ImGui::IsMouseDown(0);
    bool mouseClicked = ImGui::IsMouseClicked(0);
    bool mouseReleased = ImGui::IsMouseReleased(0);

    auto tex = global.renderer.GetMapTex();
    auto &tiles = global.context.GetTiles();

    int xtiles = tex.width / 8, 
        ytiles = tex.height / 8;
    
    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    static constexpr float scale = 3.0f;
    ImVec2 size = ImVec2(tex.width, tex.height);

    for (unsigned int y = 0; y < ytiles; ++y)
    for (unsigned int x = 0; x < xtiles; ++x)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        ImVec2 uv0 = ImVec2(x, y) * tileSize / size;
        drawList->AddImage(tex.id, pos, pos + tileSize * scale, uv0, uv0 + tileSize / size);

        ImRect bb_ = ImRect(pos, pos + tileSize * scale);
        bool hovered = ImGui::ItemHoverable(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), ImGuiItemFlags_AllowOverlap);

        if (hovered && mouseClicked)
        {
            auto &brush = global.brush;
            global.context.AddTile({x, y}, {brush.xflip, brush.yflip, (unsigned short)brush.tile, global.renderer.GetPickerPaletteNum()});
            global.renderer.Redraw();
        }
    }
}

void TilemapPane(void)
{
    if (ImGui::BeginChild("Tilemap", ImVec2(0.0f, 0.0f), 0, ImGuiWindowFlags_HorizontalScrollbar))
    {
        TilemapWindow();
        ImGui::EndChild();
    }
}