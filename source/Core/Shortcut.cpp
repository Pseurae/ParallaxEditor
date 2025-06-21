#include "Core/Shortcut.h"
#include "Core/Snapshot.h"
#include "UI/Helpers.h"
#include "Global.h"
#include <GLFW/glfw3.h>

static constexpr unsigned int sShortcutMask = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT;

static const Shortcut sShortcuts[] =
{
    { GLFW_KEY_Z, GLFW_MOD_CONTROL, action_stack_do_undo, true },
    { GLFW_KEY_Y, GLFW_MOD_CONTROL, action_stack_do_redo, true },
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