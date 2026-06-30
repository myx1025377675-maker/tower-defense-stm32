#ifndef GAME_MAP_H
#define GAME_MAP_H

#include "game_defs.h"

void GameMap_Init(GameMap *map);
uint8_t GameMap_CanPlace(GameMap *map, uint16_t col, uint16_t row);
void GameMap_PlaceTower(GameMap *map, uint16_t col, uint16_t row);
void GameMap_RemoveTower(GameMap *map, uint16_t col, uint16_t row);

#endif
