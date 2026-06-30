#include "game_ui.h"
#include "game_state.h"
#include "game_map.h"
#include "game_tower.h"
#include "game_render.h"
#include "game_score.h"
#include "lcd_rgb.h"
#include <stdio.h>
#include <string.h>

static uint16_t TextWidth(char *text, pChineseFont *font)
{
    uint16_t w = 0;
    uint8_t *p = (uint8_t *)text;
    uint16_t ascii_w = (uint16_t)(font->Width / 2U);

    while (*p) {
        if (*p <= 0x7FU) {
            w = (uint16_t)(w + ascii_w);
            p++;
        } else {
            w = (uint16_t)(w + font->Width);
            p += 2;
        }
    }
    return w;
}

static void DrawText(uint16_t x, uint16_t y, char *text, uint32_t color, pChineseFont *font)
{
    LCD_SetTextFont(font);
    LCD_SetColor(color);
    LCD_DisplayText(x, y, text);
}

static void DrawCenteredText(uint16_t y, char *text, uint32_t color, pChineseFont *font)
{
    uint16_t w = TextWidth(text, font);
    uint16_t x = (w < MAP_WIDTH) ? (uint16_t)((MAP_WIDTH - w) / 2U) : 0;
    DrawText(x, y, text, color, font);
}

static void DrawButton(uint16_t bx, uint16_t by, uint16_t bw, uint16_t bh,
                       char *text, pChineseFont *font, uint32_t bg_color, uint32_t text_color)
{
    uint16_t tw = TextWidth(text, font);
    uint16_t tx = (tw < bw) ? (uint16_t)(bx + (bw - tw) / 2U) : (uint16_t)(bx + 6U);
    uint16_t ty = (uint16_t)(by + (bh - font->Height) / 2U);

    LCD_SetColor(COLOR_BLACK);
    LCD_FillRect((uint16_t)(bx + 3U), (uint16_t)(by + 3U), bw, bh);
    LCD_SetColor(bg_color);
    LCD_FillRect(bx, by, bw, bh);
    LCD_SetColor(COLOR_WHITE);
    LCD_DrawRect(bx, by, bw, bh);
    LCD_SetBackColor(bg_color);
    DrawText(tx, ty, text, text_color, font);
}

static void DrawPanel(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char *title)
{
    LCD_SetColor(0xff173321);
    LCD_FillRect(x, y, w, h);
    LCD_SetColor(0xff2f6b45);
    LCD_DrawRect(x, y, w, h);
    LCD_DrawRect((uint16_t)(x + 2U), (uint16_t)(y + 2U), (uint16_t)(w - 4U), (uint16_t)(h - 4U));
    LCD_SetBackColor(0xff173321);
    DrawText((uint16_t)(x + 12U), (uint16_t)(y + 10U), title, COLOR_GOLD, &CH_Font16);
}

static void DrawHighScores(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    const GameScoreRecord *records = GameScore_GetRecords();
    uint8_t count = GameScore_GetCount();
    uint8_t i;
    uint8_t rank = 0;
    uint8_t displayed = 0;
    char line[64];
    char *win = "\xCA\xA4\xC0\xFB";
    char *lose = "\xCA\xA7\xB0\xDC";

    DrawPanel(x, y, w, h, "\xB8\xDF\xB7\xD6\xB0\xF1");
    LCD_SetBackColor(0xff173321);

    if (count == 0) {
        DrawText((uint16_t)(x + 18U), (uint16_t)(y + 58U),
                 "\xD4\xDD\xCE\xDE\xBC\xC7\xC2\xBC", COLOR_LIGHT_GRAY, &CH_Font16);
        return;
    }

    for (i = 0; i < count && i < GAME_SCORE_MAX_RECORDS; i++) {
        uint16_t row_y;

        if (i > 0 && records[i].score == records[i - 1].score) {
            continue;
        }

        rank++;
        row_y = (uint16_t)(y + 48U + displayed * 31U);
        sprintf(line, "\xB5\xDA%d\xC3\xFB %lu\xB7\xD6", (int)rank, (unsigned long)records[i].score);
        DrawText((uint16_t)(x + 12U), row_y, line,
                 (displayed == 0) ? COLOR_GOLD : COLOR_WHITE, &CH_Font16);
        sprintf(line, "\xB2\xA8%d %s", records[i].wave, records[i].won ? win : lose);
        DrawText((uint16_t)(x + 12U), (uint16_t)(row_y + 15U), line,
                 records[i].won ? COLOR_GREEN : COLOR_LIGHT_GRAY, &CH_Font16);
        displayed++;
    }
}

void GameUI_DrawMenu(void)
{
    LCD_SetColor(0xff0f2a1b);
    LCD_FillRect(0, 0, MAP_WIDTH, UI_PANEL_H);
    LCD_SetColor(0xff173f27);
    LCD_FillCircle(95, 75, 70);
    LCD_SetColor(0xff234f32);
    LCD_FillCircle(555, 345, 95);

    LCD_SetColor(COLOR_DARK_GRAY);
    LCD_FillRect(UI_PANEL_X, 0, UI_PANEL_W, UI_PANEL_H);
    LCD_SetColor(0xff5ea66f);
    LCD_DrawLine(UI_PANEL_X, 0, UI_PANEL_X, UI_PANEL_H);

    LCD_SetBackColor(0xff0f2a1b);
    DrawCenteredText(54, "\xCB\xFE\xB7\xC0\xCA\xD8\xCE\xC0", COLOR_GOLD, &CH_Font32);
    DrawCenteredText(108, "STM32H743  FreeRTOS", COLOR_CYAN, &CH_Font16);

    DrawButton(190, 178, 250, 62, "\xBF\xAA\xCA\xBC\xD3\xCE\xCF\xB7", &CH_Font16, COLOR_GREEN, COLOR_WHITE);

    DrawPanel(36, 270, 385, 104, "\xCB\xFE\xD0\xCE");
    LCD_SetBackColor(0xff173321);
    DrawText(55, 318, "\xB5\xE3\xBB\xF7\xB8\xF1\xD7\xD3\xBD\xA8\xCB\xFE\xA3\xAC\xB5\xE3\xBB\xF7\xCB\xFE\xD0\xCE\xC9\xFD\xBC\xB6", COLOR_WHITE, &CH_Font16);
    DrawText(55, 346, "\xD3\xD2\xB2\xE0\xD1\xA1\xD4\xF1\xCB\xFE\xD0\xCD\xA3\xAC\xCA\xD8\xD7\xA1\xD7\xEE\xBA\xF3\xD2\xBB\xB2\xA8", COLOR_LIGHT_GRAY, &CH_Font16);

    LCD_SetBackColor(0xff0f2a1b);
    DrawCenteredText(410, "\xD7\xF7\xD5\xDF\xA3\xBA\xC2\xED\xD4\xBE\xCF\xE8  \xD1\xA7\xBA\xC5\xA3\xBA" "37520232205011", COLOR_GOLD, &CH_Font16);
    DrawCenteredText(438, "\xD7\xA8\xD2\xB5\xA3\xBA\xCF\xC3\xC3\xC5\xB4\xF3\xD1\xA7\xBC\xAF\xB3\xC9\xB5\xE7\xC2\xB7\xC9\xE8\xBC\xC6\xD3\xEB\xBC\xAF\xB3\xC9\xCF\xB5\xCD\xB3", COLOR_LIGHT_GRAY, &CH_Font16);
    DrawHighScores(440, 170, 185, 204);
    DrawButton(470, 386, 125, 34, "CLEAR", &CH_Font16, COLOR_RED, COLOR_WHITE);

    /* UART: print high score leaderboard */
    {
        const GameScoreRecord *records = GameScore_GetRecords();
        uint8_t count = GameScore_GetCount();
        uint8_t i;
        printf("\r\n========== High Scores ==========\r\n");
        if (count == 0) {
            printf("  (No records yet)\r\n");
        } else {
            for (i = 0; i < count && i < 5; i++) {
                printf("  %d. %lu pts (Wave %d, %s)\r\n",
                       i + 1, (unsigned long)records[i].score,
                       records[i].wave, records[i].won ? "Win" : "Lose");
            }
        }
        printf("=================================\r\n");
    }
}

void GameUI_DrawGameOver(void)
{
    char buf[64];

    GameScore_Submit(g_game.score, g_game.current_wave, 0);

    LCD_SetColor(0xff160b0b);
    LCD_FillRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
    LCD_SetColor(0xff3c1515);
    LCD_FillCircle(115, 95, 80);

    LCD_SetBackColor(0xff160b0b);
    DrawCenteredText(74, "\xD3\xCE\xCF\xB7\xBD\xE1\xCA\xF8", COLOR_RED, &CH_Font32);

    sprintf(buf, "\xD7\xEE\xD6\xD5\xB5\xC3\xB7\xD6\xA3\xBA%d", (int)g_game.score);
    DrawCenteredText(164, buf, COLOR_YELLOW, &CH_Font16);
    sprintf(buf, "\xCD\xA8\xB9\xFD\xB2\xA8\xB4\xCE\xA3\xBA%d/%d", g_game.current_wave, TOTAL_WAVES);
    DrawCenteredText(206, buf, COLOR_WHITE, &CH_Font16);
    if (GameScore_LastSubmitWasRecord()) {
        DrawCenteredText(248, "\xD0\xC2\xBC\xCD\xC2\xBC\xA3\xA1", COLOR_GOLD, &CH_Font16);
    }

    DrawHighScores(430, 98, 195, 210);
    DrawButton(170, 325, 145, 52, "\xD6\xD8\xD0\xC2\xBF\xAA\xCA\xBC", &CH_Font16, COLOR_GREEN, COLOR_WHITE);
    DrawButton(330, 325, 145, 52, "\xD6\xF7\xB2\xCB\xB5\xA5", &CH_Font16, COLOR_BLUE, COLOR_WHITE);
}

void GameUI_DrawWin(void)
{
    char buf[64];

    GameScore_Submit(g_game.score, TOTAL_WAVES, 1);

    LCD_SetColor(0xff12321d);
    LCD_FillRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
    LCD_SetColor(0xff315c2d);
    LCD_FillCircle(510, 92, 75);

    LCD_SetBackColor(0xff12321d);
    DrawCenteredText(74, "\xCA\xA4\xC0\xFB\xA3\xA1", COLOR_GOLD, &CH_Font32);

    sprintf(buf, "\xD7\xEE\xD6\xD5\xB5\xC3\xB7\xD6\xA3\xBA%d", (int)g_game.score);
    DrawCenteredText(164, buf, COLOR_YELLOW, &CH_Font16);
    sprintf(buf, "\xC8\xAB\xB2\xBF%d\xB2\xA8\xD2\xD1\xCA\xD8\xD7\xA1\xA3\xA1", TOTAL_WAVES);
    DrawCenteredText(206, buf, COLOR_WHITE, &CH_Font16);
    if (GameScore_LastSubmitWasRecord()) {
        DrawCenteredText(248, "\xD0\xC2\xBC\xCD\xC2\xBC\xA3\xA1", COLOR_GOLD, &CH_Font16);
    }

    DrawHighScores(430, 98, 195, 210);
    DrawButton(170, 325, 145, 52, "\xD6\xD8\xD0\xC2\xBF\xAA\xCA\xBC", &CH_Font16, COLOR_GREEN, COLOR_WHITE);
    DrawButton(330, 325, 145, 52, "\xD6\xF7\xB2\xCB\xB5\xA5", &CH_Font16, COLOR_BLUE, COLOR_WHITE);
}

void GameUI_DrawWaveBreak(void)
{
    char buf[64];

    LCD_SetBackColor(COLOR_DARK_GREEN);
    sprintf(buf, "\xB5\xDA%d\xB2\xA8\xCD\xEA\xB3\xC9\xA3\xA1", g_game.current_wave);
    DrawCenteredText(200, buf, COLOR_GOLD, &CH_Font16);

    sprintf(buf, "\xBD\xF0\xB1\xD2\xA3\xBA%d\xA3\xA8\xBD\xB1\xC0\xF8+50\xA3\xA9", (int)g_game.gold);
    DrawCenteredText(250, buf, COLOR_YELLOW, &CH_Font16);

    DrawButton(220, 300, 200, 50, "\xCF\xC2\xD2\xBB\xB2\xA8", &CH_Font16, COLOR_GREEN, COLOR_WHITE);
}

uint8_t GameUI_GridFromPixel(uint16_t px, uint16_t py, uint16_t *col, uint16_t *row)
{
    if (px >= MAP_WIDTH || py >= MAP_HEIGHT) return 0;
    *col = px / CELL_SIZE;
    *row = py / CELL_SIZE;
    return 1;
}

uint8_t GameUI_IsInGameArea(uint16_t px, uint16_t py)
{
    return (px < MAP_WIDTH && py < MAP_HEIGHT);
}

static uint8_t InRect(uint16_t px, uint16_t py, uint16_t bx, uint16_t by, uint16_t bw, uint16_t bh)
{
    return (px >= bx && px <= bx + bw && py >= by && py <= by + bh);
}

void GameUI_ProcessInput(UIEvent *evt)
{
    uint16_t col, row;

    if (evt->event_type != 1) return;

    switch (g_game.state) {

    case GAME_STATE_MENU:
        if (InRect(evt->x, evt->y, 190, 178, 250, 62)) {
            GameState_Reset();
            g_game.state = GAME_STATE_PLAYING;
            GameState_StartWave();
            return;
        }
        if (InRect(evt->x, evt->y, 470, 386, 125, 34)) {
            GameScore_Clear();
            return;
        }
        break;

    case GAME_STATE_PLAYING:
    case GAME_STATE_PAUSED:
        {
            if (InRect(evt->x, evt->y, HUD_EXIT_BTN_X, HUD_BTN_Y, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H)) {
                g_game.selected_tower_type = 0xFF;
                g_game.selected_tower_id = 0xFF;
                g_game.wave_active = 0;
                g_game.state = GAME_STATE_MENU;
                return;
            }
            if (InRect(evt->x, evt->y, HUD_PAUSE_BTN_X, HUD_BTN_Y, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H)) {
                if (g_game.state == GAME_STATE_PLAYING)
                    g_game.state = GAME_STATE_PAUSED;
                else if (g_game.state == GAME_STATE_PAUSED)
                    g_game.state = GAME_STATE_PLAYING;
                return;
            }
        }

        if (g_game.state == GAME_STATE_PAUSED) break;

        if (evt->x >= UI_PANEL_X) {
            uint8_t saved_tower_id = g_game.selected_tower_id;
            uint8_t i;

            for (i = 0; i < TOWER_TYPE_COUNT; i++) {
                uint16_t by = (uint16_t)(60 + i * 100);
                if (InRect(evt->x, evt->y, UI_PANEL_X + 5, by, UI_PANEL_W - 10, 85)) {
                    g_game.selected_tower_type = i;
                    g_game.selected_tower_id = 0xFF;
                    return;
                }
            }

            if (saved_tower_id < MAX_TOWERS &&
                g_game.towers[saved_tower_id].active) {
                uint16_t uy = 370 + 55;
                if (InRect(evt->x, evt->y, UI_PANEL_X + 10, uy, 60, 20)) {
                    Tower *t = &g_game.towers[saved_tower_id];
                    uint16_t upg_cost = (uint16_t)((uint32_t)g_game.tower_types[t->type].upgrade_cost * t->level);
                    if (g_game.gold >= upg_cost && t->level < 3) {
                        g_game.gold -= upg_cost;
                        Tower_Upgrade(t, g_game.tower_types);
                        printf("[Tower] %s upgraded to Lv.%d (cost: %d)\r\n",
                               g_game.tower_types[t->type].name, t->level, upg_cost);
                    }
                    return;
                }
            }

            if (saved_tower_id < MAX_TOWERS &&
                g_game.towers[saved_tower_id].active) {
                uint16_t sy = 370 + 78;
                if (InRect(evt->x, evt->y, UI_PANEL_X + 10, sy, 60, 20)) {
                    uint16_t val = Tower_GetSellValue(&g_game.towers[saved_tower_id]);
                    uint8_t sold_type = g_game.towers[saved_tower_id].type;
                    g_game.gold += val;
                    printf("[Tower] %s sold (+%d gold)\r\n",
                           g_game.tower_types[sold_type].name, val);
                    Tower_Sell(g_game.towers, saved_tower_id, &g_game.map);
                    g_game.selected_tower_id = 0xFF;
                    return;
                }
            }

            g_game.selected_tower_type = 0xFF;
            g_game.selected_tower_id = 0xFF;
            return;
        }

        if (GameUI_GridFromPixel(evt->x, evt->y, &col, &row)) {
            uint8_t i;
            for (i = 0; i < MAX_TOWERS; i++) {
                Tower *t = &g_game.towers[i];
                if (t->active && t->grid_x == col && t->grid_y == row) {
                    g_game.selected_tower_id = i;
                    g_game.selected_tower_type = 0xFF;
                    return;
                }
            }

            if (g_game.selected_tower_type < TOWER_TYPE_COUNT) {
                TowerType *tt = &g_game.tower_types[g_game.selected_tower_type];
                if (g_game.gold >= tt->cost) {
                    if (GameMap_CanPlace(&g_game.map, col, row)) {
                        if (Tower_Build(g_game.towers, g_game.selected_tower_type, col, row, g_game.tower_types)) {
                            GameMap_PlaceTower(&g_game.map, col, row);
                            g_game.gold -= tt->cost;
                            g_game.selected_tower_type = 0xFF;
                        }
                    }
                }
            }
            if (g_game.selected_tower_type >= TOWER_TYPE_COUNT)
                g_game.selected_tower_id = 0xFF;
        }
        break;

    case GAME_STATE_WAVE_BREAK:
        if (InRect(evt->x, evt->y, HUD_EXIT_BTN_X, HUD_BTN_Y, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H)) {
            g_game.selected_tower_type = 0xFF;
            g_game.selected_tower_id = 0xFF;
            g_game.wave_active = 0;
            g_game.state = GAME_STATE_MENU;
            return;
        }
        if (InRect(evt->x, evt->y, 220, 300, 200, 50)) {
            g_game.state = GAME_STATE_PLAYING;
            GameState_StartWave();
            return;
        }
        break;

    case GAME_STATE_OVER:
    case GAME_STATE_WIN:
        if (InRect(evt->x, evt->y, 170, 325, 145, 52)) {
            GameState_Reset();
            g_game.state = GAME_STATE_PLAYING;
            GameState_StartWave();
            return;
        }
        if (InRect(evt->x, evt->y, 330, 325, 145, 52)) {
            g_game.state = GAME_STATE_MENU;
            return;
        }
        break;

    default:
        break;
    }
}
