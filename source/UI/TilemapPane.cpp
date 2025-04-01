#include "UI/TilemapPane.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "Global.h"
#include <iostream>
#include "Core/Snapshot.h"

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
    auto &brush = global.brush;
    const auto &tiles = global.context.GetTiles();

    auto tex = global.renderer.GetMapTex();

    int xtiles = global.context.GetWidth(), 
        ytiles = global.context.GetHeight();

    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    float scale = 4.0f * global.zoomScale;

    ImVec2 size = ImVec2(tex.width, tex.height);

    bool hasHovered = false;
    ImVec2 hoveredPos = ImVec2(0, 0);

    static ImVec2 sStartDrag = ImVec2(0, 0);
    static int sBrushWidth = 1, sBrushHeight = 1;

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
            if (ImGui::IsMouseClicked(1))
                sStartDrag = ImVec2(x, y);

            hasHovered = true;
            hoveredPos = ImVec2(x, y);
        }
    }

    static Snapshot snapshotBuffer;
    if (hasHovered)
    {
        if (ImGui::IsMouseClicked(0))
        {
            snapshotBuffer.oldTiles = global.context.GetTiles();
        }

        if (ImGui::IsMouseReleased(0))
        {
            snapshotBuffer.newTiles = global.context.GetTiles();
            action_stack_add_undo_action(snapshotBuffer);
        }

        if (ImGui::IsMouseDown(0))
        {
            ApplyTiles(hoveredPos.x, hoveredPos.y);
        }

        if (ImGui::IsMouseDown(1))
        {
            auto delta = ImGui::GetMouseDragDelta(1);
            sBrushWidth = std::max<int>(std::abs(delta.x) / (tileSize.x * scale), 0) + 1;
            sBrushHeight = std::max<int>(std::abs(delta.y) / (tileSize.y * scale), 0) + 1;

            ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + sStartDrag * tileSize * scale;
            drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(sBrushWidth, sBrushHeight) * tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
        }
        else
        {
            ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + hoveredPos * tileSize * scale;
            drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(global.brush.width, global.brush.height) * tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
        }

        if (ImGui::IsMouseReleased(1))
        {
            brush.fromTileset = false;
            brush.width = sBrushWidth;
            brush.height = sBrushHeight;

            brush.selection.resize(sBrushWidth * sBrushHeight);

            for (int y = 0; y < sBrushHeight; ++y)
            for (int x = 0; x < sBrushWidth; ++x)
            {
                TilePosition pos = {(unsigned int)(sStartDrag.x + x), (unsigned int)(sStartDrag.y + y)};
                brush.selection[x + y * sBrushWidth] = tiles.contains(pos) ? tiles.at(pos) : global.context.GetDefaultTile();
            }
        }
    }

    ImVec2 widgetsize = ImVec2(xtiles, ytiles) * tileSize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void TilemapPane(void)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size - ImVec2(0.0f, ImGui::GetFrameHeight()));

    static constexpr ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("###ParallaxEditor", NULL, sWindowFlags | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginChild("Tilemap", ImVec2(0, 0), 0, sWindowFlags))
    {
        TilemapWindow();
        ImGui::EndChild();
    }
    ImGui::End();
}