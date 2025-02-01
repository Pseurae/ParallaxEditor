#pragma once

struct Shortcut
{
    int key;
    int mods;
    void (*func)(void);
};

void shortcut_callback(int key, int mods);