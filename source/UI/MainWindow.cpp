#include "UI/MainWindow.h"
#include "UI/TilesetPane.h"
#include "UI/TilemapPane.h"
#include "Global.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "UI/Popups/Prompt.h"

void MainWindow(void)
{
    if (global.context.IsLoaded())
    {
        TilesetPane();
        ImGui::SameLine();
        TilemapPane();
    }
    else
    {
        ImGui::Text("Press Ctrl+N to create a new tilemap.\nPress Ctrl+O to open an existing tilemap.");
    }
}
