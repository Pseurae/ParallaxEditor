#pragma once

struct Shortcut
{
    int key;
    int mods;
    void (*func)(void);
    bool allowHold = false;
};

void shortcut_callback(int key, int mods, int action);