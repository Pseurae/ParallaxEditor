#pragma once

struct PlaceTileAction
{
    int i;
    unsigned short oldTile, newTile;
};

struct Action;
using ActionFunc = void(*)(const Action &, bool undo);

struct Action
{
    ActionFunc func;

    union 
    {
        PlaceTileAction placeTile;
    };
};

void action_stack_clear(void);
bool action_stack_can_undo(void);
bool action_stack_can_redo(void);
void action_stack_add_undo_action(Action action);
void action_stack_do_undo(void);
void action_stack_do_redo(void);

Action action_place_tile_new(int i, unsigned short oldTile, unsigned short newTile);
