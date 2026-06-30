#ifndef GAME_TOWER_H
#define GAME_TOWER_H

#include "game_defs.h"

void TowerTypes_Init(TowerType *types);
int8_t Tower_FindFree(Tower *towers);
uint8_t Tower_Build(Tower *towers, uint8_t type, uint16_t col, uint16_t row, TowerType *types);
uint8_t Tower_Upgrade(Tower *t, TowerType *types);
void Tower_Sell(Tower *towers, uint8_t id, GameMap *map);
uint16_t Tower_GetSellValue(Tower *t);
void Tower_FindTarget(Tower *t, Enemy *enemies);

#endif
