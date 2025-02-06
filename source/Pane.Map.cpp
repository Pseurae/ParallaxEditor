#include "Pane.Picker.h"
#include "ActionStack.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "Global.h"

static void right_click(unsigned short i)
{
    // global.brush.fromTileset = false;
    // global.brush.xflip = (i & Mask::FlipX) == Mask::FlipX;
    // global.brush.yflip = (i & Mask::FlipY) == Mask::FlipY;
    // global.brush.tile = i & Mask::Index;
    
    // global.brush.palette = (i >> 12) & 0xF;
    // renderer_change_palette(global.renderer, global.brush.palette);
}

static unsigned short change_tile_palette(unsigned short tile, unsigned char palette)
{
    static constexpr unsigned int Mask = 0xF << 12;
    tile &= ~Mask;
    tile |= (palette & 0xF) << 12;
    return tile;
}

static void left_click(unsigned int startX, unsigned int startY)
{
    std::vector<unsigned short> oldTiles(global.brush.width, global.brush.height);

    for (int y = 0; y < global.brush.height; ++y)
    for (int x = 0; x < global.brush.width; ++x)
    {
        if ((startX + x) >= 32 || (startY + y) >= 32) continue;

        unsigned int i = (startX + x) + (startY + y) * 32;

        global.tilemap[i] = global.brush.fromTileset ? 
            change_tile_palette(global.brush.selection[x + y * global.brush.width], global.brush.palette) : 
            global.brush.selection[x + y * global.brush.width];
    }
}

static void tilemap_window(void)
{
    auto drawList = ImGui::GetWindowDrawList();

    ImTextureID tex_id = (ImTextureID)(uintptr_t)global.renderer.mapFinalTex;

    static float zoom = 1.0f;

    float scale = 4.0f * global.zoomScale;
    ImVec2 size = ImVec2(256, 256);
    ImVec2 tilesize = ImVec2(8, 8);

    static constexpr int sTilesInRow = 32;

    bool has_hovered = false;
    int hovered_item;

    static bool changed_tile = true;
    static int last_hovered_item;
    static int sStartDrag = 0;
    static int sWidth = 1, sHeight = 1;

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

            if (ImGui::IsMouseClicked(1)) 
            {
                sStartDrag = i; 
                global.brush.fromTileset = false;
            }

            // if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) 
            //     right_click(global.tilemap[i]);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) 
            {
                if (changed_tile) left_click(x, y);
                changed_tile = false;
            }
        }
    }

    if (has_hovered)
    {
        if (ImGui::IsMouseDown(1))
        {
            auto delta = ImGui::GetMouseDragDelta();
            sWidth = std::max<int>(delta.x / (tilesize.x * scale), 0) + 1;
            sHeight = std::max<int>(delta.y / (tilesize.y * scale), 0) + 1;

            int x = sStartDrag % 32, y = sStartDrag / 32;
            ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
            drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(sWidth, sHeight) * tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
        }
        else
        {
            int x = hovered_item % 32, y = hovered_item / 32;
            ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(0.5f, 0.5f) + ImVec2(x, y) * tilesize * scale;
            drawList->AddRect(pos - ImVec2(0.5f, 0.5f), pos + ImVec2(global.brush.width, global.brush.height) * tilesize * scale + ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, 255));
        }
    }

    if (has_hovered && ImGui::IsMouseReleased(1))
    {
        auto &brush = global.brush;

        brush.selection.resize(sWidth * sHeight);

        for (int y = 0; y < sHeight; ++y)
        for (int x = 0; x < sWidth; ++x)
        {
            int startX = sStartDrag % 32, 
                startY = sStartDrag / 32;
            
            uint16_t idx = (startX + x) + (startY + y) * 32;
            brush.selection[x + y * sWidth] = global.tilemap[idx];
        }

        brush.width = sWidth;
        brush.height = sHeight;
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