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
    if (global.context.IsProjectLoaded())
        OptionsPane();

    if (global.context.IsMapLoaded())
    {
        TilesetPane();
        TilemapPane();
    }
}
