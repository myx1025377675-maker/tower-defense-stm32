#include "game_score.h"
#include "qspi_w25q64.h"
#include <string.h>

extern QSPI_HandleTypeDef hqspi;

#define SCORE_FLASH_ADDR     0x007FF000UL
#define SCORE_MAGIC          0x54445331UL  /* "TDS1" */
#define SCORE_VERSION        1U

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    GameScoreRecord records[GAME_SCORE_MAX_RECORDS];
    uint32_t checksum;
} GameScoreStorage;

static GameScoreStorage g_score_store;
static uint8_t g_score_loaded;
static uint8_t g_last_record;

static uint32_t ScoreChecksum(const GameScoreStorage *store)
{
    const uint8_t *p = (const uint8_t *)store;
    uint32_t sum = 0;
    uint32_t n = (uint32_t)((const uint8_t *)&store->checksum - p);

    while (n--) {
        sum = (sum << 5) - sum + *p++;
    }
    return sum;
}

static void ScoreDefaults(void)
{
    memset(&g_score_store, 0, sizeof(g_score_store));
    g_score_store.magic = SCORE_MAGIC;
    g_score_store.version = SCORE_VERSION;
    g_score_store.checksum = ScoreChecksum(&g_score_store);
}

static uint8_t ScoreIsValid(const GameScoreStorage *store)
{
    if (store->magic != SCORE_MAGIC) return 0;
    if (store->version != SCORE_VERSION) return 0;
    if (store->count > GAME_SCORE_MAX_RECORDS) return 0;
    return (ScoreChecksum(store) == store->checksum);
}

static void ScoreSave(void)
{
    int8_t status;

    g_score_store.magic = SCORE_MAGIC;
    g_score_store.version = SCORE_VERSION;
    g_score_store.checksum = ScoreChecksum(&g_score_store);

    /* Leave memory-mapped mode briefly so QSPI can erase/write, then restore it for fonts. */
    HAL_QSPI_Abort(&hqspi);
    status = QSPI_W25Qxx_Init();
    if (status == QSPI_W25Qxx_OK) {
        QSPI_W25Qxx_SectorErase(SCORE_FLASH_ADDR);
        QSPI_W25Qxx_WriteBuffer((uint8_t *)&g_score_store, SCORE_FLASH_ADDR, sizeof(g_score_store));
        QSPI_W25Qxx_MemoryMappedMode();
    }
}

void GameScore_Init(void)
{
    const GameScoreStorage *mapped =
        (const GameScoreStorage *)(W25Qxx_Mem_Addr + SCORE_FLASH_ADDR);

    memcpy(&g_score_store, mapped, sizeof(g_score_store));
    if (!ScoreIsValid(&g_score_store)) {
        ScoreDefaults();
    }

    g_score_loaded = 1;
    g_last_record = 0;
}

uint8_t GameScore_Submit(uint32_t score, uint8_t wave, uint8_t won)
{
    GameScoreRecord rec;
    uint8_t insert_at;
    uint8_t i;

    if (!g_score_loaded) {
        GameScore_Init();
    }

    rec.score = score;
    rec.wave = wave;
    rec.won = won;
    g_last_record = 0;

    insert_at = g_score_store.count;
    for (i = 0; i < g_score_store.count; i++) {
        if (score > g_score_store.records[i].score) {
            insert_at = i;
            break;
        }
    }

    if (insert_at >= GAME_SCORE_MAX_RECORDS) {
        return 0;
    }

    if (g_score_store.count < GAME_SCORE_MAX_RECORDS) {
        g_score_store.count++;
    }

    for (i = (uint8_t)(g_score_store.count - 1); i > insert_at; i--) {
        g_score_store.records[i] = g_score_store.records[i - 1];
    }

    g_score_store.records[insert_at] = rec;
    g_last_record = 1;
    ScoreSave();
    return 1;
}

uint8_t GameScore_GetCount(void)
{
    return g_score_store.count;
}

const GameScoreRecord *GameScore_GetRecords(void)
{
    return g_score_store.records;
}

uint8_t GameScore_LastSubmitWasRecord(void)
{
    return g_last_record;
}

void GameScore_Clear(void)
{
    if (!g_score_loaded) {
        GameScore_Init();
    }

    ScoreDefaults();
    g_last_record = 0;
    ScoreSave();
}
