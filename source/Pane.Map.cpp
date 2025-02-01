#include "Pane.Picker.h"
#include "ActionStack.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "Global.h"

static void right_click(unsigned short i)
{
    global.brush.xflip = (i & Mask::FlipX) == Mask::FlipX;
    global.brush.yflip = (i & Mask::FlipY) == Mask::FlipY;
    global.brush.tile = i & Mask::Index;
    
    global.brush.palette = (i >> 12) & 0xF;
    renderer_change_palette(global.renderer, global.brush.palette);
}

static void left_click(unsigned int i)
{
    uint16_t newTile = (
        global.brush.tile & Mask::Index |
        ((global.brush.palette & 0xF) << 12) |
        (global.brush.xflip ? Mask::FlipX : 0) |
        (global.brush.yflip ? Mask::FlipY : 0)
    );

    action_stack_add_undo_action(
        action_place_tile_new(i, global.tilemap[i], newTile));
    global.tilemap[i] = newTile;
}

static void tilemap_window(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.mapFinalTex;

    static float zoom = 1.0f;

    ImVec2 scale = ImVec2(4, 4) * global.zoomScale;
    ImVec2 size = ImVec2(256, 256);
    ImVec2 tilesize = ImVec2(8, 8);

    static constexpr int sTilesInRow = 32;

    bool has_hovered = false;
    int hovered_item;

    static bool changed_tile = true;
    static int last_hovered_item;

    for (int i = 0; i < 1024; ++i)
    {
        int x = i % sTilesInRow, y = i / sTilesInRow;
        ImVec2 uv0 = ImVec2(i % 32, i / 32) * tilesize / size;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddImage(tex_id, pos, pos + tilesize * scale, uv0, uv0 + tilesize / size);

        ImRect bb_ = ImRect(pos, pos + tilesize * scale);
        bool hovered = ImGui::ItemHoverable(bb_, ImGui::GetCurrentContext()->CurrentWindow->GetIDFromRectangle(bb_), ImGuiItemFlags_AllowOverlap);

        if (hovered)
        {
            has_hovered = true;
            hovered_item = i;

            if (last_hovered_item != hovered_item)
                changed_tile = true;
            
            last_hovered_item = i;

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) 
                right_click(global.tilemap[i]);
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) 
            {
                if (changed_tile) left_click(i);
                changed_tile = false;
            }
        }
    }

    if (has_hovered)
    {
        int x = hovered_item % 32, y = hovered_item / 32;
        ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    if (global.drawScreenBounds)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + tilesize * scale * ImVec2(30.0f, 20.0f) + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
    }

    ImVec2 widgetsize = ImVec2(sTilesInRow, static_cast<int>(1024 / sTilesInRow)) * tilesize;
    ImRect bb(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + widgetsize * scale + ImVec2(1.0f, 1.0f));

    ImGui::ItemSize(bb);
    ImGui::ItemAdd(bb, 0);
}

#include <iostream>

void tilemap_pane(void)
{
    if (ImGui::BeginChild("Tilemap", ImVec2(0.0f, 0.0f), 0, ImGuiWindowFlags_HorizontalScrollbar))
    {
        tilemap_window();
        ImGui::EndChild();
    }
}