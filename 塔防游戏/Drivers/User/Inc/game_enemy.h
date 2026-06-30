#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

#include "game_defs.h"

void EnemyTypes_Init(EnemyType *types);
uint8_t Enemy_Spawn(Enemy *enemies, uint8_t type, uint16_t spawn_delay, GameMap *map, EnemyType *types);
void Enemy_Move(Enemy *enemies, GameMap *map, uint32_t dt_ms);
uint8_t Enemy_ReachedEnd(Enemy *e, GameMap *map);
void Enemy_ApplySlow(Enemy *e, uint16_t pct, uint16_t duration_ms, uint32_t now_ms);
uint8_t Enemy_CountActive(Enemy *enemies);
uint8_t Enemy_CountPending(Enemy *enemies);

#endif
