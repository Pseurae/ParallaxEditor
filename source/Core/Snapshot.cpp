#include "Core/Snapshot.h"
#include "Global.h"

static std::list<Snapshot> sRedoStack;
static std::list<Snapshot> sUndoStack;

void action_stack_clear(void)
{
    sUndoStack.clear();
    sRedoStack.clear();
}

bool action_stack_can_undo(void) { return !sUndoStack.empty(); }
bool action_stack_can_redo(void) { return !sRedoStack.empty(); }

void action_stack_add_undo_action(Snapshot snapshot)
{
    sUndoStack.push_back(snapshot);
}

void action_stack_do_undo(void)
{
    if (!action_stack_can_undo()) 
        return;

    Snapshot snapshot = sUndoStack.back();
    sUndoStack.pop_back();

    global.context.SetTiles(snapshot.oldTiles);
    global.renderer.Redraw();
    sRedoStack.push_back(snapshot);
}

void action_stack_do_redo(void)
{
    if (!action_stack_can_redo()) 
        return;

    Snapshot snapshot = sRedoStack.back();
    sRedoStack.pop_back();

    global.context.SetTiles(snapshot.newTiles);
    global.renderer.Redraw();
    sUndoStack.push_back(snapshot);
}