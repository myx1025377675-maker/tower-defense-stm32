#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include "game_defs.h"

#define GAME_SCORE_MAX_RECORDS 5

typedef struct {
    uint32_t score;
    uint8_t  wave;
    uint8_t  won;
} GameScoreRecord;

void GameScore_Init(void);
uint8_t GameScore_Submit(uint32_t score, uint8_t wave, uint8_t won);
uint8_t GameScore_GetCount(void);
const GameScoreRecord *GameScore_GetRecords(void);
uint8_t GameScore_LastSubmitWasRecord(void);
void GameScore_Clear(void);

#endif /* GAME_SCORE_H */
