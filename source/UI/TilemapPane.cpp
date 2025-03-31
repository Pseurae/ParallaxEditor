#include "UI/TilemapPane.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "Global.h"
#include <iostream>

static void ApplyTiles(unsigned int startX, unsigned int startY)
{
    auto &brush = global.brush;

    for (unsigned int y = 0; y < brush.height; ++y)
    for (unsigned int x = 0; x < brush.width; ++x)
        global.context.AddTile({x + startX, y + startY}, brush.selection[x + y * brush.width]);

    global.renderer.Redraw();
}

static void TilemapWindow(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    auto tex = global.renderer.GetMapTex();
    auto &tiles = global.context.GetTiles();

    int xtiles = global.context.GetWidth(), 
        ytiles = global.context.GetHeight();

    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    float scale = 4.0f * global.zoomScale;

    ImVec2 size = ImVec2(tex.width, tex.height);

    bool hasHovered = false;
    ImVec2 hoveredPos = ImVec2(0, 0);

    for (unsigned int y = 0; y < ytiles; ++y)
    for (unsigned int x = 0; x < xtiles; ++x)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        ImVec2 uv0 = ImVec2(x, y) * tileSize / size;
        drawList->AddImage(tex.id, pos, pos + tileSize * scale, uv0, uv0 + tileSize / size);

        ImRect bb_ = ImRect(pos, pos + tileSize * scale);
        bool hovered = ImGui::ItemHoverable(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), ImGuiItemFlags_AllowOverlap);
        if (hovered)
        {
            hasHovered = true;
            hoveredPos = ImVec2(x, y);
        }
    }

    if (hasHovered)
    {
        if (ImGui::IsMouseDown(0))
            ApplyTiles(hoveredPos.x, hoveredPos.y);

        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + hoveredPos * tileSize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(global.brush.width, global.brush.height) * tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
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