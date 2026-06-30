#ifndef GAME_UI_H
#define GAME_UI_H

#include "game_defs.h"

void GameUI_DrawMenu(void);
void GameUI_DrawGameOver(void);
void GameUI_DrawWin(void);
void GameUI_DrawWaveBreak(void);
void GameUI_ProcessInput(UIEvent *evt);
uint8_t GameUI_GridFromPixel(uint16_t px, uint16_t py, uint16_t *col, uint16_t *row);
uint8_t GameUI_IsInGameArea(uint16_t px, uint16_t py);

#endif
