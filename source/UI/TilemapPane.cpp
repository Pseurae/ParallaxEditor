#include "UI/TilemapPane.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "Global.h"
#include <iostream>
#include "Core/Snapshot.h"
#include "UI/Helpers.h"

ImRect GetSelectionRectFromDrag(ImVec2 start, ImVec2 end, const ImVec2 &tileSize);

static void ApplyTiles(unsigned int startX, unsigned int startY)
{
    auto &brush = global.brush;

    for (unsigned int y = 0; y < brush.height; ++y)
    for (unsigned int x = 0; x < brush.width; ++x)
    {
        Tile tile = brush.selection.at(x + y * brush.width);
        tile.xflip ^= brush.xflip;
        tile.yflip ^= brush.yflip;

        int modifiedX = brush.xflip ? (brush.width - x - 1) : x;
        int modifiedY = brush.yflip ? (brush.height - y - 1) : y;
        global.context.AddTile({modifiedX + startX, modifiedY + startY}, tile);
    }

    global.renderer.Redraw();
}

static void TilemapWindow(void)
{
    auto drawList = ImGui::GetWindowDrawList();
    auto &brush = global.brush;
    const auto &tiles = global.context.GetTiles();

    auto &metatiles = global.renderer.GetMetatileTex();

    auto tex = global.renderer.GetMapTex();

    int xtiles = global.context.GetWidth() * 2, 
        ytiles = global.context.GetHeight() * 2;

    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    float scale = 4.0f * global.zoomScale;

    ImVec2 size = ImVec2(tex.width, tex.height);

    bool hasHovered = false;
    ImVec2 hoveredPos = ImVec2(0, 0);

    static ImVec2 sStartDrag = ImVec2(0, 0), sEndDrag = ImVec2(1, 1);

    drawList->AddImage(metatiles.id, ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(metatiles.width, metatiles.height) * scale);

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
                sStartDrag = ImGui::GetMousePos() - ImGui::GetCursorScreenPos();

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
            sEndDrag = ImGui::GetMousePos() - ImGui::GetCursorScreenPos();
            ImRect selectionBox = GetSelectionRectFromDrag(sStartDrag, sEndDrag, tileSize * scale);
            drawList->AddRect(selectionBox.Min * tileSize * scale + ImGui::GetCursorScreenPos(), selectionBox.Max * tileSize * scale + ImGui::GetCursorScreenPos() + ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
        }
        else
        {
            ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + hoveredPos * tileSize * scale;
            drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(global.brush.width, global.brush.height) * tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
        }

        if (ImGui::IsMouseReleased(1))
        {
            brush.fromTileset = false;
            ImRect selectionBox = GetSelectionRectFromDrag(sStartDrag, sEndDrag, tileSize * scale);
            ImVec2 selectionSize = selectionBox.GetSize();

            brush.width = selectionSize.x;
            brush.height = selectionSize.y;

            brush.xflip = false;
            brush.yflip = false;

            brush.selection.resize(brush.width * brush.height);

            for (int y = 0; y < brush.height; ++y)
            for (int x = 0; x < brush.width; ++x)
            {
                TilePosition pos = {(unsigned int)(selectionBox.Min.x + x), (unsigned int)(selectionBox.Min.y + y)};
                brush.selection[x + y * brush.width] = tiles.contains(pos) ? tiles.at(pos) : global.context.GetDefaultTile();

                if (brush.height == 1 && brush.width == 1)
                {
                    Tile &tile = brush.selection[x + y * brush.width];
                    brush.xflip = tile.xflip;
                    brush.yflip = tile.yflip;

                    tile.xflip = false;
                    tile.yflip = false;

                    global.renderer.SetPickerPaletteNum(tile.palette);
                }
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

    static constexpr ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("###ParallaxEditor", NULL, sWindowFlags | ImGuiWindowFlags_MenuBar);
    ImGui::BeginChild("Tilemap", ImVec2(0, 0), 0, sWindowFlags);
    TilemapWindow();
    ImGui::EndChild();
    ImGui::End();
}