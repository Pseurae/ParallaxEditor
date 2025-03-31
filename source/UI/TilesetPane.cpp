#include "UI/MainWindow.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

void TilesetSelector(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    bool mouseDown = ImGui::IsMouseDown(0);
    bool mouseClicked = ImGui::IsMouseClicked(0);
    bool mouseReleased = ImGui::IsMouseReleased(0);
    auto tex = global.renderer.GetPickerTex();

    static constexpr int tilesInRow = 16;
    static constexpr ImVec2 tileSize = ImVec2(8, 8);
    static constexpr float scale = 3.0f;

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

        if (hovered && mouseClicked)
            global.brush.tile = i;
    }

    ImVec2 widgetsize = ImVec2(tilesInRow, 1024 / tilesInRow) * tileSize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    {
        int x = global.brush.tile % tilesInRow, y = global.brush.tile / tilesInRow;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tileSize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tileSize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void TilesetPane(void)
{
    if (ImGui::BeginChild("Tileset", ImVec2(500.0, 0.0)))
    {
        ImGui::Checkbox("X Flip", &global.brush.xflip);
        ImGui::Checkbox("Y Flip", &global.brush.yflip);

        int palNum = global.renderer.GetPickerPaletteNum();
        if (ImGui::InputInt("Palette", &palNum))
            global.renderer.SetPickerPaletteNum(std::min(std::max(palNum, 0), 15));

        ImGui::Spacing();

        if (ImGui::BeginChild("###TilesetSelector", ImVec2(400.0f, 0.0f)))
        {
            TilesetSelector();
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
}