#include "ActionStack.h"
#include "Global.h"

#define NUM_ACTIONS 25

static Action sRedoStack[NUM_ACTIONS];
static Action sUndoStack[NUM_ACTIONS];

static unsigned int sRedoIdx = 0;
static unsigned int sUndoIdx = 0;

void action_stack_clear(void)
{
    sRedoIdx = sUndoIdx = 0;
}

bool action_stack_can_undo(void) { return sUndoIdx; }
bool action_stack_can_redo(void) { return sRedoIdx; }

void action_stack_add_undo_action(Action action)
{
    sUndoStack[sUndoIdx++] = action;
}

void action_stack_do_undo(void)
{
    if (!sUndoIdx) return;
    Action action = sUndoStack[--sUndoIdx];
    action.func(action, true);
    sRedoStack[sRedoIdx++] = action;
}

void action_stack_do_redo(void)
{
    if (!sRedoIdx) return;
    Action action = sRedoStack[--sRedoIdx];
    action.func(action, false);
    sUndoStack[sUndoIdx++] = action;
}

static void action_place_tile_func(const Action &action, bool undo)
{
    PlaceTileAction ptAction = action.placeTile;
    global.tilemap[ptAction.i] = undo ? ptAction.oldTile : ptAction.newTile;
}

Action action_place_tile_new(int i, unsigned short oldTile, unsigned short newTile)
{
    return (Action){ .func = action_place_tile_func, .placeTile = { i, oldTile, newTile } };
}