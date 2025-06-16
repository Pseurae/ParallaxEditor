#pragma once

#include <vector>
#include <string>
#include "Utils/Tile.h"

void CreateNewTilemap(int, int);
void TryLoadTilemap(void);
void TrySaveTilemap(void);
std::vector<Tile> LoadBinaryTilemap(const std::string &path);
void TryImportTilemap(const std::vector<Tile> &tiles, int width);
void TryExportTilemap(void);
void TryLoadPrimaryTileset(const std::string &p);
void TryLoadSecondaryTileset(const std::string &p);
void UnloadAllPalettes(void);
void OpenPalette(unsigned int slot);
void OpenPaletteFolder(const std::string &paletteFolderPath);
void TryLoadUnderlay(void);
void TryOpenProjectFolder(void);
