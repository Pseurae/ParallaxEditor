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
    auto underlay = global.renderer.GetUnderlayTex();

    auto tex = global.renderer.GetMapTex();

    int xtiles = global.context.GetWidth(), 
        ytiles = global.context.GetHeight();

    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    float scale = 4.0f * global.zoomScale;

    ImVec2 size = ImVec2(tex.width, tex.height);

    bool hasHovered = false;
    ImVec2 hoveredPos = ImVec2(0, 0);

    drawList->AddImage(underlay.id, ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(underlay.width, underlay.height) * scale);

    for (unsigned int y = 0; y < ytiles; ++y)
    for (unsigned int x = 0; x < xtiles; ++x)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        ImVec2 uv0 = ImVec2(x, y) * tileSize / size;
        drawList->AddImage(tex.id, pos, pos + tileSize * scale, uv0, uv0 + tileSize / size, ImColor(1.0f, 1.0f, 1.0f, global.transparency));

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
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size - ImVec2(0.0f, ImGui::GetFrameHeight()));

    static constexpr ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("###ParallaxEditor", NULL, sWindowFlags | ImGuiWindowFlags_MenuBar);
    TilemapWindow();
    ImGui::End();
}