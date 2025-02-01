#include "Shortcut.h"
#include "Utils.h"
#include "ActionStack.h"

#include <GLFW/glfw3.h>

static constexpr unsigned int sShortcutMask = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT;

static const Shortcut sShortcuts[] =
{
    { GLFW_KEY_O, GLFW_MOD_CONTROL, open_tilemap },
    { GLFW_KEY_O, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT, open_palettes },
    { GLFW_KEY_1, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT, open_primary_tileset },
    { GLFW_KEY_2, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT, open_secondary_tileset },
    { GLFW_KEY_Z, GLFW_MOD_CONTROL, action_stack_do_undo, true },
    { GLFW_KEY_Y, GLFW_MOD_CONTROL, action_stack_do_redo, true },
    { GLFW_KEY_S, GLFW_MOD_CONTROL, save_tilemap },
    { GLFW_KEY_S, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT, save_as_tilemap },
};

void shortcut_callback(int key, int mods, int action)
{
    mods &= sShortcutMask;

    for (int i = 0; i < sizeof(sShortcuts) / sizeof(Shortcut); ++i)
    {
        auto s = sShortcuts[i];
        if ((action == GLFW_PRESS || (s.allowHold && action == GLFW_REPEAT)) && 
            s.key == key && (mods & s.mods) == s.mods && (mods & s.mods) == mods)
        {
            s.func();
            break;
        }
    }
}

