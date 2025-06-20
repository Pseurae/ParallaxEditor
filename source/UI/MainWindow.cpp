#include "UI/MainWindow.h"
#include "UI/OptionsPane.h"
#include "UI/TilesetPane.h"
#include "UI/TilemapPane.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "UI/Popups/Prompt.h"

void MainWindow(void)
{
    auto &tex = global.renderer.GetMetatileTex();
    ImGui::Image(tex.id, ImVec2(tex.width, tex.height) * 6);
    // if (global.context.IsLoaded())
    // {
    //     TilesetPane();
    //     OptionsPane();
    //     TilemapPane();
    // }
}
