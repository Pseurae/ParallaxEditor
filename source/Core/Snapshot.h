#pragma once

#include <unordered_map>
#include "Utils/Tile.h"

struct Snapshot
{
    std::unordered_map<TilePosition, Tile> oldTiles;
    std::unordered_map<TilePosition, Tile> newTiles;
};

void action_stack_clear(void);
bool action_stack_can_undo(void);
bool action_stack_can_redo(void);
void action_stack_add_undo_action(Snapshot action);
void action_stack_do_undo(void);
void action_stack_do_redo(void);