#include "Pane.Picker.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "Global.h"

static void tileset_selector(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    bool mouseDown = ImGui::IsMouseDown(0);
    bool mouseClicked = ImGui::IsMouseClicked(0);
    bool mouseReleased = ImGui::IsMouseReleased(0);

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.pickerFinalTex;
    float scale = 3.0f;
    ImVec2 size = ImVec2(128, 512);
    ImVec2 tilesize = ImVec2(8, 8);

    static constexpr int sTilesInRow = 16;
    static int sStartDrag = 0;
    static int sWidth = 1, sHeight = 1;
    bool anyHovered = false; 

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % sTilesInRow, y = i / sTilesInRow;
        ImVec2 uv0 = ImVec2(i % 16, i / 16) * tilesize / size;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddImage(tex_id, pos, pos + tilesize * scale, uv0, uv0 + tilesize / size);

        ImRect bb_ = ImRect(pos, pos + tilesize * scale);
        auto id_ = ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_);
        bool hovered = ImGui::ItemHoverable(bb_, id_, 0);
        // bool pressed = ImGui::ButtonBehavior(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), &hovered, &held);

        if (hovered)
        {
            if (mouseClicked) 
            {
                sStartDrag = i; 
                global.brush.fromTileset = true;
            }

            anyHovered = true;
        } 
    }

    if (anyHovered && mouseDown)
    {
        auto delta = ImGui::GetMouseDragDelta();
        sWidth = std::max<int>(delta.x / (tilesize.x * scale), 0) + 1;
        sHeight = std::max<int>(delta.y / (tilesize.y * scale), 0) + 1;
    }

    if (anyHovered && mouseReleased)
    {
        auto &brush = global.brush;

        brush.selection.resize(sWidth * sHeight);

        for (int y = 0; y < sHeight; ++y)
        for (int x = 0; x < sWidth; ++x)
        {
            int startX = sStartDrag % sTilesInRow, 
                startY = sStartDrag / sTilesInRow;
            
            uint16_t idx = (startX + x) + (startY + y) * sTilesInRow;

            uint16_t tile = (
                idx & Mask::Index |
                ((global.brush.palette & 0xF) << 12) |
                (global.brush.xflip ? Mask::FlipX : 0) |
                (global.brush.yflip ? Mask::FlipY : 0)
            );

            brush.selection[x + y * sWidth] = tile;
        }

        brush.width = sWidth;
        brush.height = sHeight;
    }

    ImVec2 widgetsize = ImVec2(sTilesInRow, static_cast<int>(1024 / sTilesInRow)) * tilesize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    int x = sStartDrag % sTilesInRow, y = sStartDrag / sTilesInRow;
    ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
    if (global.brush.fromTileset)
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(sWidth, sHeight) * tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void tileset_pane(void)
{
    if (ImGui::BeginChild("Tileset", ImVec2(400.0f, 0.0f)))
    {
        // ImGui::Text("ID: 0x%X", global.brush.tile);

        ImGui::Spacing();

        ImGui::Checkbox("X Flip", &global.brush.xflip); ImGui::SameLine();
        ImGui::Checkbox("Y Flip", &global.brush.yflip);

        ImGui::Spacing();

        int palettenum = global.brush.palette;
        if (ImGui::InputInt("Palette", &palettenum))
        {
            if (palettenum < 0) palettenum = 0;
            else if (palettenum > 15) palettenum = 15;
            global.brush.palette = palettenum;

            renderer_change_palette(global.renderer, global.brush.palette);
        }

        ImGui::Spacing();

        if (ImGui::BeginChild("###TilesetSelector", ImVec2(400.0f, 0.0f)))
        {
            tileset_selector();
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
}