#include "UI/MainWindow.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

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
    static int sStartDrag = 0;
    static int sBrushWidth = 1, sBrushHeight = 1;

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
                sStartDrag = i;
                global.brush.fromTileset = true;
            }

            hasHovered = true;
        }
    }

    if (hasHovered)
    {
        if (mouseDown)
        {
            auto delta = ImGui::GetMouseDragDelta();
            sBrushWidth = std::max<int>(std::abs(delta.x) / (tileSize.x * scale), 0) + 1;
            sBrushHeight = std::max<int>(std::abs(delta.y) / (tileSize.y * scale), 0) + 1;
        }

        if (mouseReleased)
        {
            brush.fromTileset = true;
            brush.width = sBrushWidth;
            brush.height = sBrushHeight;

            brush.selection.resize(sBrushWidth * sBrushHeight);

            for (int y = 0; y < sBrushHeight; ++y)
            for (int x = 0; x < sBrushWidth; ++x)
            {
                int startX = sStartDrag % tilesInRow, 
                    startY = sStartDrag / tilesInRow;
                
                Tile tile;
                tile.id = (startX + x) + (startY + y) * tilesInRow;
                tile.palette = global.renderer.GetPickerPaletteNum();

                brush.selection[x + y * sBrushWidth] = tile;
            }
        }
    }

    if (global.brush.fromTileset)
    {
        int x = sStartDrag % tilesInRow, y = sStartDrag / tilesInRow;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(sBrushWidth, sBrushHeight) * tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    ImVec2 widgetsize = ImVec2(tilesInRow, 1024 / tilesInRow) * tileSize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void TilesetPane(void)
{
    ImGui::Begin("Tileset", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::Checkbox("X Flip", &global.brush.xflip);
    ImGui::SameLine();
    ImGui::Checkbox("Y Flip", &global.brush.yflip);

    int palNum = global.renderer.GetPickerPaletteNum();
    if (ImGui::InputInt("Palette", &palNum))
        global.renderer.SetPickerPaletteNum(std::min(std::max(palNum, 0), 15));

    ImGui::Spacing();

    ImGui::BeginChild("###TilesetSelector", ImVec2(400.0f, 800.0f));
    TilesetSelector();
    ImGui::EndChild();
    ImGui::End();
}