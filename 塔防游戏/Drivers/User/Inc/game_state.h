#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "game_defs.h"

void GameState_Init(void);
void GameState_Reset(void);
void GameState_StartWave(void);
void GameState_Update(uint32_t dt_ms);
void GameState_Hurt(uint8_t damage);

/* Wave definitions */
void Waves_Init(Wave *waves);

#endif
