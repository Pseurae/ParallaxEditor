#include "Pane.Picker.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "Global.h"

static void tileset_selector(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.pickerFinalTex;
    ImVec2 scale = ImVec2(3, 3);
    ImVec2 size = ImVec2(128, 512);
    ImVec2 tilesize = ImVec2(8, 8);

    static int sTilesInRow = 16;

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % sTilesInRow, y = i / sTilesInRow;
        ImVec2 uv0 = ImVec2(i % 16, i / 16) * tilesize / size;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddImage(tex_id, pos, pos + tilesize * scale, uv0, uv0 + tilesize / size);

        ImRect bb_ = ImRect(pos, pos + tilesize * scale);
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), &hovered, &held);

        if (held || pressed)
            global.brush.tile = i;
    }

    ImVec2 widgetsize = ImVec2(sTilesInRow, static_cast<int>(1024 / sTilesInRow)) * tilesize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(global.brush.tile % sTilesInRow, static_cast<int>(global.brush.tile / sTilesInRow)) * tilesize * scale;
    drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

void tileset_pane(void)
{
    if (ImGui::BeginChild("Tileset", ImVec2(400.0f, 0.0f)))
    {
        ImGui::Text("ID: 0x%X", global.brush.tile);

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