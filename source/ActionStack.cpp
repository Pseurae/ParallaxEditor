#include "ActionStack.h"
#include "Global.h"
#include <list>

static std::list<Action> sRedoStack;
static std::list<Action> sUndoStack;

void action_stack_clear(void)
{
    sUndoStack.clear();
    sRedoStack.clear();
}

bool action_stack_can_undo(void) { return !sUndoStack.empty(); }
bool action_stack_can_redo(void) { return !sRedoStack.empty(); }

void action_stack_add_undo_action(Action action)
{
    sUndoStack.push_back(action);
}

void action_stack_do_undo(void)
{
    if (!action_stack_can_undo()) 
        return;

    Action action = sUndoStack.back();
    sUndoStack.pop_back();

    action.func(action, true);
    sRedoStack.push_back(action);
}

void action_stack_do_redo(void)
{
    if (!action_stack_can_redo()) 
        return;

    Action action = sRedoStack.back();
    sRedoStack.pop_back();

    action.func(action, false);
    sUndoStack.push_back(action);
}

static void action_place_tile_func(const Action &action, bool undo)
{
    // PlaceTileAction ptAction = action.placeTile;
    // global.tilemap[ptAction.i] = undo ? ptAction.oldTile : ptAction.newTile;
}

Action action_place_tile_new(int i, std::vector<unsigned short> oldTiles, std::vector<unsigned short> newTiles, unsigned int width, unsigned int height)
{
    return (Action){ .func = action_place_tile_func };
}