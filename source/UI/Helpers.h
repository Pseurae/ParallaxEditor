#pragma once

#include <vector>
#include <string>
#include "Utils/Tile.h"

void CreateNewTilemap(int, int);
std::vector<Tile> LoadBinaryTilemap(const std::string &path);
void TryImportTilemap(const std::vector<Tile> &tiles, int width);
void TryExportTilemap(void);
void TryLoadPrimaryTileset(const std::string &p);
void TryLoadSecondaryTileset(const std::string &p);
void UnloadAllPalettes(void);
void OpenPalette(unsigned int slot);
void OpenPaletteFolder(const std::string &paletteFolderPath, int start, int end);
void TryOpenProjectFolder(void);
void TryOpenPrimaryMetatiles(void);
void TryOpenMap(const std::string &mapName);