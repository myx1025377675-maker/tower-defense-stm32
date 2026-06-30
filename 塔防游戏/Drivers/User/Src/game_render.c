#include "game_render.h"
#include "game_state.h"
#include "game_tower.h"
#include "game_enemy.h"
#include "lcd_rgb.h"
#include <stdio.h>
#include <string.h>

static void RenderText(uint16_t x, uint16_t y, char *text, uint32_t color, pChineseFont *font)
{
    LCD_SetTextFont(font);
    LCD_SetColor(color);
    LCD_DisplayText(x, y, text);
}

/* Draw a single grid cell */
static void DrawCell(uint16_t col, uint16_t row, uint32_t color)
{
    uint16_t x = col * CELL_SIZE;
    uint16_t y = row * CELL_SIZE;

    LCD_SetColor(color);
    LCD_FillRect(x + 1, y + 1, CELL_SIZE - 2, CELL_SIZE - 2);
}

void GameRender_Map(void)
{
    uint8_t r, c;
    uint32_t bg, path_color, blocked_color;
    uint16_t x1, y1;

    /* Draw background */
    LCD_SetColor(COLOR_DARK_GREEN);
    LCD_FillRect(0, 0, MAP_WIDTH, MAP_HEIGHT);

    /* Draw cells */
    for (r = 0; r < GRID_ROWS; r++) {
        for (c = 0; c < GRID_COLS; c++) {
            x1 = c * CELL_SIZE;
            y1 = r * CELL_SIZE;

            switch (g_game.map.grid[r][c]) {
                case CELL_PATH:
                    path_color = COLOR_BROWN;
                    LCD_SetColor(path_color);
                    LCD_FillRect(x1 + 2, y1 + 2, CELL_SIZE - 4, CELL_SIZE - 4);
                    break;
                case CELL_TOWER:
                    bg = COLOR_DARK_GREEN;
                    LCD_SetColor(bg);
                    LCD_FillRect(x1 + 2, y1 + 2, CELL_SIZE - 4, CELL_SIZE - 4);
                    break;
                case CELL_BLOCKED:
                    blocked_color = COLOR_GRAY;
                    LCD_SetColor(blocked_color);
                    LCD_FillRect(x1 + 2, y1 + 2, CELL_SIZE - 4, CELL_SIZE - 4);
                    break;
                default:
                    break;
            }

            /* Grid lines */
            LCD_SetColor(0x1942);
            LCD_DrawRect(x1, y1, CELL_SIZE, CELL_SIZE);
        }
    }

    /* Draw start marker */
    {
        uint16_t sx = g_game.map.waypoint_x[0];
        uint16_t sy = g_game.map.waypoint_y[0];
        LCD_SetColor(COLOR_GREEN);
        LCD_FillRect(sx - 8, sy - 8, 16, 16);
        LCD_SetColor(COLOR_WHITE);
        LCD_SetFont(&Font12);
        LCD_DisplayChar(sx - 3, sy - 6, 'S');
    }

    /* Draw exit marker */
    {
        uint8_t last = g_game.map.waypoint_count - 1;
        uint16_t ex = g_game.map.waypoint_x[last];
        uint16_t ey = g_game.map.waypoint_y[last];
        LCD_SetColor(COLOR_RED);
        LCD_FillRect(ex - 8, ey - 8, 16, 16);
        LCD_SetColor(COLOR_WHITE);
        LCD_SetFont(&Font12);
        LCD_DisplayChar(ex - 3, ey - 6, 'E');
    }

    /* Waypoint debug dots and connecting lines */
    {
        uint8_t wi;
        for (wi = 0; wi < g_game.map.waypoint_count; wi++) {
            uint16_t wx = g_game.map.waypoint_x[wi];
            uint16_t wy = g_game.map.waypoint_y[wi];
            LCD_SetColor(COLOR_YELLOW);
            LCD_FillCircle(wx, wy, 3);
            if (wi > 0) {
                LCD_SetColor(COLOR_CYAN);
                LCD_DrawLine(g_game.map.waypoint_x[wi - 1],
                             g_game.map.waypoint_y[wi - 1], wx, wy);
            }
        }
    }
}

void GameRender_Towers(void)
{
    uint8_t i;
    for (i = 0; i < MAX_TOWERS; i++) {
        Tower *t = &g_game.towers[i];
        if (!t->active) continue;

        int16_t cx = (int16_t)t->px;
        int16_t cy = (int16_t)t->py;

        /* Tower base */
        if (t->type == TOWER_ARROW) {
            LCD_SetColor(COLOR_YELLOW);
        } else if (t->type == TOWER_CANNON) {
            LCD_SetColor(COLOR_ORANGE);
        } else {
            LCD_SetColor(COLOR_SKY_BLUE);
        }
        LCD_FillCircle((uint16_t)cx, (uint16_t)cy, 14);

        /* Level indicator */
        LCD_SetColor(COLOR_BLACK);
        {
            char lvl[4];
            lvl[0] = '0' + t->level;
            lvl[1] = '\0';
            LCD_SetFont(&Font12);
            LCD_DisplayString((uint16_t)(cx - 3), (uint16_t)(cy - 6), lvl);
        }

        /* Attack range ring (subtle) */
        if (t->target_id < MAX_ENEMIES) {
            LCD_SetColor(COLOR_RED);
        } else {
            LCD_SetColor(COLOR_DARK_GRAY);
        }
        LCD_DrawCircle((uint16_t)cx, (uint16_t)cy, t->range);

        /* Selected highlight */
        if (g_game.selected_tower_id == i) {
            LCD_SetColor(COLOR_WHITE);
            LCD_DrawRect((uint16_t)(cx - 16), (uint16_t)(cy - 16), 33, 33);
        }
    }
}

void GameRender_Enemies(void)
{
    uint8_t i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &g_game.enemies[i];
        if (!e->active || e->hp <= 0 || e->spawn_delay > 0) continue;

        uint16_t ex = (uint16_t)e->x;
        uint16_t ey = (uint16_t)e->y;
        uint16_t r;

        /* Enemy body size based on type */
        if (e->type == ENEMY_FAST) r = 8;
        else if (e->type == ENEMY_TANK) r = 12;
        else r = 10;

        LCD_SetColor(e->color);
        LCD_FillCircle(ex, ey, r);

        /* Slowed indicator */
        if (e->slow_until > g_game.game_tick_ms) {
            LCD_SetColor(COLOR_SKY_BLUE);
            LCD_DrawCircle(ex, ey, r + 2);
        }

        /* Health bar */
        {
            uint16_t bar_w = 20;
            uint16_t bar_h = 3;
            int16_t bar_x = (int16_t)ex - bar_w / 2;
            int16_t bar_y = (int16_t)ey - (int16_t)r - 6;
            uint16_t hp_w;

            hp_w = (uint16_t)((uint32_t)bar_w * (uint32_t)(e->hp > 0 ? e->hp : 0) / (uint32_t)e->max_hp);

            LCD_SetColor(COLOR_RED);
            LCD_FillRect((uint16_t)bar_x, (uint16_t)bar_y, bar_w, bar_h);
            LCD_SetColor(COLOR_GREEN);
            LCD_FillRect((uint16_t)bar_x, (uint16_t)bar_y, hp_w, bar_h);
        }
    }
}

void GameRender_Projectiles(void)
{
    uint8_t i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &g_game.projectiles[i];
        if (!p->active) continue;

        LCD_SetColor(p->color);
        LCD_FillCircle((uint16_t)p->x, (uint16_t)p->y, 3);
    }
}

void GameRender_InfoBar(void)
{
    char buf[32];
    uint16_t y = INFO_BAR_Y;
    uint16_t bar_w;

    /* Background */
    LCD_SetColor(COLOR_DARK_GRAY);
    LCD_FillRect(0, y, MAP_WIDTH, INFO_BAR_H);

    /* Separator line */
    LCD_SetColor(COLOR_WHITE);
    LCD_DrawLine(0, y, MAP_WIDTH, y);

    LCD_SetBackColor(COLOR_DARK_GRAY);
    LCD_SetFont(&Font16);

    /* Lives */
    RenderText(5, y + 5, "\xC9\xFA\xC3\xFC", COLOR_RED, &CH_Font16);
    LCD_SetFont(&Font16);
    LCD_SetColor(COLOR_WHITE);
    LCD_DisplayNumber(52, y + 5, g_game.lives, 2);

    /* Gold */
    RenderText(102, y + 5, "\xBD\xF0\xB1\xD2", COLOR_GOLD, &CH_Font16);
    LCD_SetFont(&Font16);
    LCD_SetColor(COLOR_WHITE);
    LCD_DisplayNumber(150, y + 5, g_game.gold, 4);

    /* Wave */
    RenderText(225, y + 5, "\xB2\xA8\xB4\xCE", COLOR_CYAN, &CH_Font16);
    LCD_SetFont(&Font16);
    LCD_SetColor(COLOR_WHITE);
    LCD_DisplayNumber(273, y + 5, (int32_t)(g_game.current_wave + 1), 1);
    LCD_DisplayChar(283, y + 5, '/');
    LCD_DisplayNumber(293, y + 5, (int32_t)TOTAL_WAVES, 1);

    /* Score */
    RenderText(345, y + 5, "\xB5\xC3\xB7\xD6", COLOR_YELLOW, &CH_Font16);
    LCD_SetFont(&Font16);
    LCD_SetColor(COLOR_WHITE);
    LCD_DisplayNumber(393, y + 5, g_game.score, 5);

    /* Status line */
    LCD_SetFont(&Font12);
    if (g_game.state == GAME_STATE_WAVE_BREAK) {
        RenderText(5, y + 30, "\xCF\xC2\xD2\xBB\xB2\xA8\xD7\xBC\xB1\xB8\xD6\xD0", COLOR_GREEN, &CH_Font16);
    } else if (g_game.wave_active && g_game.wave_pre_delay_timer > 0) {
        sprintf(buf, "\xB5\xDA%d\xB2\xA8\xC0\xB4\xCF\xAE\xA3\xBA%d\xC3\xEB",
                g_game.current_wave + 1, g_game.wave_pre_delay_timer / 1000 + 1);
        RenderText(5, y + 30, buf, COLOR_YELLOW, &CH_Font16);
    }

    /* Pause/resume and exit buttons */
    {
        uint16_t px_btn = HUD_PAUSE_BTN_X;
        uint16_t ex_btn = HUD_EXIT_BTN_X;
        uint16_t py_btn = HUD_BTN_Y;
        if (g_game.state == GAME_STATE_PAUSED) {
            LCD_SetColor(COLOR_GREEN);
            LCD_FillRect(px_btn, py_btn, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H);
            LCD_SetBackColor(COLOR_GREEN);
            RenderText(px_btn + 13, py_btn + 7, "\xBC\xCC\xD0\xF8", COLOR_BLACK, &CH_Font16);
        } else {
            LCD_SetColor(COLOR_YELLOW);
            LCD_FillRect(px_btn, py_btn, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H);
            LCD_SetBackColor(COLOR_YELLOW);
            RenderText(px_btn + 13, py_btn + 7, "\xD4\xDD\xCD\xA3", COLOR_BLACK, &CH_Font16);
        }
        LCD_SetColor(COLOR_WHITE);
        LCD_DrawRect(px_btn, py_btn, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H);

        LCD_SetColor(COLOR_ORANGE);
        LCD_FillRect(ex_btn, py_btn, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H);
        LCD_SetBackColor(COLOR_ORANGE);
        RenderText(ex_btn + 13, py_btn + 7, "\xCD\xCB\xB3\xF6", COLOR_BLACK, &CH_Font16);
        LCD_SetColor(COLOR_WHITE);
        LCD_DrawRect(ex_btn, py_btn, HUD_ACTION_BTN_W, HUD_ACTION_BTN_H);
    }

    /* Wave progress bar */
    if (g_game.wave_active || g_game.state == GAME_STATE_WAVE_BREAK) {
        uint16_t bar_x = 5;
        uint16_t bar_yy = y + 55;
        uint16_t total_w = MAP_WIDTH - 10;

        LCD_SetColor(COLOR_DARK_GRAY);
        LCD_FillRect(bar_x, bar_yy, total_w, 10);
        LCD_SetColor(COLOR_GREEN);

        if (g_game.wave_active) {
            uint8_t total_enemies = 0, killed = 0;
            {
                Wave *w = &g_game.waves[g_game.current_wave];
                uint8_t ei;
                for (ei = 0; ei < w->entry_count; ei++)
                    total_enemies += w->entries[ei].count;
            }
            killed = (total_enemies > 0) ? (uint8_t)(total_enemies - Enemy_CountActive(g_game.enemies) - Enemy_CountPending(g_game.enemies)) : 0;
            if (total_enemies > 0) {
                bar_w = (uint16_t)((uint32_t)total_w * killed / total_enemies);
            } else {
                bar_w = 0;
            }
            LCD_FillRect(bar_x, bar_yy, bar_w, 10);
        }
        LCD_SetColor(COLOR_WHITE);
        LCD_DrawRect(bar_x, bar_yy, total_w, 10);
    }
}

void GameRender_UIPanel(void)
{
    uint16_t x = UI_PANEL_X;
    TowerType *tt;
    uint16_t by;

    /* Panel background */
    LCD_SetColor(COLOR_DARK_GRAY);
    LCD_FillRect(x, 0, UI_PANEL_W, UI_PANEL_H);

    /* Separator line */
    LCD_SetColor(COLOR_WHITE);
    LCD_DrawLine(x, 0, x, UI_PANEL_H);

    /* Title */
    LCD_SetBackColor(COLOR_DARK_GRAY);
    RenderText(x + 28, 8, "\xCB\xFE\xD0\xCE", COLOR_CYAN, &CH_Font16);

    /* Tower buttons */
    for (uint8_t i = 0; i < TOWER_TYPE_COUNT; i++) {
        tt = &g_game.tower_types[i];
        by = (uint16_t)(60 + i * 100);

        /* Button background */
        uint32_t btn_color;
        if (g_game.selected_tower_type == i) {
            btn_color = COLOR_WHITE;
        } else if (g_game.gold >= tt->cost) {
            btn_color = COLOR_DARK_GRAY;
        } else {
            btn_color = COLOR_GRAY;
        }

        LCD_SetColor(btn_color);
        LCD_FillRect(x + 5, by, UI_PANEL_W - 10, 85);

        /* Tower colored icon */
        LCD_SetColor(tt->name_color);
        LCD_FillCircle(x + 30, by + 25, 18);

        { char lvl[4];
          lvl[0] = '0' + tt->level;
          lvl[1] = '\0';
          LCD_SetColor(COLOR_BLACK);
          LCD_SetFont(&Font12);
          LCD_DisplayString(x + 27, by + 20, lvl);
        }

        /* Tower name */
        LCD_SetBackColor(btn_color);
        RenderText(x + 55, by + 8, tt->name, tt->name_color, &CH_Font16);

        /* Stats */
        {
            char stat[32];
            sprintf(stat, "\xC9\xCB\xBA\xA6:%d", tt->damage);
            RenderText(x + 55, by + 28, stat, COLOR_WHITE, &CH_Font16);
            sprintf(stat, "\xB7\xB6\xCE\xA7:%d", tt->range);
            RenderText(x + 55, by + 43, stat, COLOR_WHITE, &CH_Font16);
            sprintf(stat, "%.1fs", (float)tt->attack_speed / 1000.0f);
            LCD_SetFont(&Font12);
            LCD_SetColor(COLOR_WHITE);
            LCD_DisplayString(x + 112, by + 58, stat);
        }
        if (tt->splash > 0) {
            RenderText(x + 55, by + 58, "\xBD\xA6\xC9\xE4", COLOR_ORANGE, &CH_Font16);
        }
        if (tt->slow_pct > 0) {
            RenderText(x + 55, by + 58, "\xBC\xF5\xCB\xD9", COLOR_SKY_BLUE, &CH_Font16);
        }

        /* Cost */
        LCD_SetColor(g_game.gold >= tt->cost ? COLOR_GOLD : COLOR_RED);
        LCD_SetFont(&Font12);
        LCD_DisplayNumber(x + 10, by + 62, (int32_t)tt->cost, 3);
        RenderText(x + 40, by + 62, "\xBD\xF0", g_game.gold >= tt->cost ? COLOR_GOLD : COLOR_RED, &CH_Font16);
    }

    /* Selected tower info */
    if (g_game.selected_tower_id < MAX_TOWERS &&
        g_game.towers[g_game.selected_tower_id].active) {
        GameRender_TowerInfo();
    }
}

void GameRender_TowerInfo(void)
{
    uint16_t x = UI_PANEL_X;
    Tower *t = &g_game.towers[g_game.selected_tower_id];
    uint16_t by = 370;

    /* Info box */
    LCD_SetColor(COLOR_BLACK);
    LCD_FillRect(x + 5, by, UI_PANEL_W - 10, 100);

    LCD_SetBackColor(COLOR_BLACK);
    RenderText(x + 10, by + 5, g_game.tower_types[t->type].name, COLOR_YELLOW, &CH_Font16);

    {
        char stat[32];
        sprintf(stat, "Lv.%d \xC9\xCB\xBA\xA6:%d", t->level, t->damage);
        RenderText(x + 10, by + 22, stat, COLOR_WHITE, &CH_Font16);
        sprintf(stat, "\xB7\xB6\xCE\xA7:%d", t->range);
        RenderText(x + 10, by + 38, stat, COLOR_WHITE, &CH_Font16);
    }

    /* Upgrade button */
    if (t->level < 3) {
        uint16_t upg_cost = (uint16_t)((uint32_t)g_game.tower_types[t->type].upgrade_cost * t->level);
        LCD_SetColor(g_game.gold >= upg_cost ? COLOR_GREEN : COLOR_RED);
        LCD_FillRect(x + 10, by + 55, 60, 20);
        LCD_SetBackColor(g_game.gold >= upg_cost ? COLOR_GREEN : COLOR_RED);
        RenderText(x + 20, by + 59, "\xC9\xFD\xBC\xB6", COLOR_WHITE, &CH_Font16);
        LCD_SetColor(COLOR_GOLD);
        LCD_DisplayNumber(x + 73, by + 57, (int32_t)upg_cost, 3);
    }

    /* Sell button */
    {
        uint16_t sell_val = Tower_GetSellValue(t);
        LCD_SetColor(COLOR_ORANGE);
        LCD_FillRect(x + 10, by + 78, 60, 20);
        LCD_SetBackColor(COLOR_ORANGE);
        RenderText(x + 20, by + 82, "\xB3\xF6\xCA\xDB", COLOR_WHITE, &CH_Font16);
        LCD_SetColor(COLOR_GOLD);
        LCD_DisplayNumber(x + 73, by + 80, (int32_t)sell_val, 3);
    }
}

void GameRender_PauseOverlay(void)
{
    LCD_SetColor(COLOR_BLACK);
    LCD_FillRect(120, 100, 400, 200);
    LCD_SetColor(COLOR_WHITE);
    LCD_DrawRect(118, 98, 404, 204);
    LCD_DrawRect(120, 100, 400, 200);

    LCD_SetBackColor(COLOR_BLACK);
    RenderText(256, 155, "\xD2\xD1\xD4\xDD\xCD\xA3", COLOR_YELLOW, &CH_Font32);
    RenderText(214, 220, "\xB5\xE3\xBB\xF7\xBC\xCC\xD0\xF8\xB7\xB5\xBB\xD8\xD5\xBD\xB3\xA1", COLOR_WHITE, &CH_Font16);
    RenderText(230, 248, "\xD3\xD2\xCF\xC2\xBD\xC7\xBF\xC9\xCD\xCB\xB3\xF6", COLOR_ORANGE, &CH_Font16);
}

void GameRender_All(void)
{
    GameRender_Map();
    GameRender_Towers();
    GameRender_Enemies();
    GameRender_Projectiles();
    GameRender_InfoBar();
    GameRender_UIPanel();

    if (g_game.state == GAME_STATE_PAUSED) {
        GameRender_PauseOverlay();
    }
}
