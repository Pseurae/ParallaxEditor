#include "UI/MainWindow.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>

ImRect GetSelectionRectFromDrag(ImVec2 start, ImVec2 end, const ImVec2 &tileSize);

void TilesetSelector(void)
{
    auto drawList = ImGui::GetWindowDrawList();
    auto tex = global.renderer.GetPickerTex();
    auto &brush = global.brush;

    bool mouseDown = ImGui::IsMouseDown(0);
    bool mouseClicked = ImGui::IsMouseClicked(0);
    bool mouseReleased = ImGui::IsMouseReleased(0);

    static constexpr int tilesInRow = 16;
    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    static constexpr float scale = 3.0f;

    bool hasHovered = false;
    static ImVec2 sStartDrag = ImVec2(0, 0), sEndDrag = ImVec2(1, 1);

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % tilesInRow, y = i / tilesInRow;
        int xtiles = (tex.width / 8);

        ImVec2 uv0 = ImVec2(i % xtiles, i / xtiles) * tileSize / ImVec2(tex.width, tex.height);
        ImVec2 uv1 = uv0 + tileSize / ImVec2(tex.width, tex.height);

        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        drawList->AddImage(tex.id, pos, pos + tileSize * scale, uv0, uv1);

        ImRect bb_ = ImRect(pos, pos + tileSize * scale);
        auto id_ = ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_);
        bool hovered = ImGui::ItemHoverable(bb_, id_, 0);

        if (hovered)
        {
            if (mouseClicked)
            {
                sStartDrag = ImGui::GetMousePos() - ImGui::GetCursorScreenPos();
                global.brush.fromTileset = true;
            }

            hasHovered = true;
        }
    }

    if (hasHovered)
    {
        if (mouseDown)
            sEndDrag = ImGui::GetMousePos() - ImGui::GetCursorScreenPos();

        if (mouseReleased)
        {
            brush.fromTileset = true;
            ImRect selectionBox = GetSelectionRectFromDrag(sStartDrag, sEndDrag, tileSize * scale);
            ImVec2 selectionSize = selectionBox.GetSize();

            brush.width = selectionSize.x;
            brush.height = selectionSize.y;

            brush.selection.resize(brush.width * brush.height);

            for (int y = 0; y < brush.height; ++y)
            for (int x = 0; x < brush.width; ++x)
            {
                Tile tile;
                tile.id = (int(selectionBox.Min.x) + x) + (int(selectionBox.Min.y) + y) * tilesInRow;
                tile.palette = global.renderer.GetPickerPaletteNum();

                brush.selection[x + y * brush.width] = tile;
            }
        }
    }

    if (brush.fromTileset)
    {
        ImRect selectionBox = GetSelectionRectFromDrag(sStartDrag, sEndDrag, tileSize * scale);
        drawList->AddRect(selectionBox.Min * tileSize * scale + ImGui::GetCursorScreenPos(), selectionBox.Max * tileSize * scale + ImGui::GetCursorScreenPos() + ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
    }
    else if (brush.width == 1 && brush.height == 1)
    {
        const Tile &tile = global.brush.selection[0]; 
        int x = tile.id % tilesInRow, y = tile.id / tilesInRow;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    ImVec2 widgetsize = ImVec2(tilesInRow, 1024 / tilesInRow) * tileSize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void TilesetPane(void)
{
    ImGui::Begin("Tileset", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("X Flip", &global.brush.xflip);
    ImGui::SameLine();
    ImGui::Checkbox("Y Flip", &global.brush.yflip);

    int palNum = global.renderer.GetPickerPaletteNum();
    if (ImGui::InputInt("Palette", &palNum))
        global.renderer.SetPickerPaletteNum(std::clamp(palNum, 0, 15));

    ImGui::Spacing();

    ImGui::BeginChild("###TilesetSelector", ImVec2(400.0f, 800.0f));
    TilesetSelector();
    ImGui::EndChild();
    ImGui::End();
}