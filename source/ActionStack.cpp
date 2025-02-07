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

    memcpy(global.tilemap, action.oldTiles, sizeof(global.tilemap));
    sRedoStack.push_back(action);
}

void action_stack_do_redo(void)
{
    if (!action_stack_can_redo()) 
        return;

    Action action = sRedoStack.back();
    sRedoStack.pop_back();

    memcpy(global.tilemap, action.newTiles, 32 * 32 * sizeof(unsigned short));
    sUndoStack.push_back(action);
}
