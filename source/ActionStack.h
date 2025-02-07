#pragma once

#include <vector>

struct Action;
using ActionFunc = void(*)(const Action &, bool undo);

struct Action
{
    unsigned short oldTiles[32 * 32], newTiles[32 * 32];
};

void action_stack_clear(void);
bool action_stack_can_undo(void);
bool action_stack_can_redo(void);
void action_stack_add_undo_action(Action action);
void action_stack_do_undo(void);
void action_stack_do_redo(void);
