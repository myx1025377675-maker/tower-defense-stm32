#include "game_tower.h"
#include "game_map.h"
#include <string.h>
#include <math.h>

void TowerTypes_Init(TowerType *types)
{
    /* Arrow Tower */
    types[TOWER_ARROW].type          = TOWER_ARROW;
    types[TOWER_ARROW].cost          = 50;
    types[TOWER_ARROW].damage        = 15;
    types[TOWER_ARROW].range         = 100;
    types[TOWER_ARROW].attack_speed  = 600;
    types[TOWER_ARROW].splash        = 0;
    types[TOWER_ARROW].slow_pct      = 0;
    types[TOWER_ARROW].slow_duration = 0;
    types[TOWER_ARROW].upgrade_cost  = 40;
    types[TOWER_ARROW].level         = 1;
    types[TOWER_ARROW].name_color    = COLOR_YELLOW;
    { char *n = "\xBC\xFD\xCB\xFE"; memcpy(types[TOWER_ARROW].name, n, 5); }

    /* Cannon Tower */
    types[TOWER_CANNON].type          = TOWER_CANNON;
    types[TOWER_CANNON].cost          = 100;
    types[TOWER_CANNON].damage        = 40;
    types[TOWER_CANNON].range         = 80;
    types[TOWER_CANNON].attack_speed  = 1200;
    types[TOWER_CANNON].splash        = 30;
    types[TOWER_CANNON].slow_pct      = 0;
    types[TOWER_CANNON].slow_duration = 0;
    types[TOWER_CANNON].upgrade_cost  = 75;
    types[TOWER_CANNON].level         = 1;
    types[TOWER_CANNON].name_color    = COLOR_ORANGE;
    { char *n = "\xC5\xDA\xCB\xFE"; memcpy(types[TOWER_CANNON].name, n, 5); }

    /* Ice Tower */
    types[TOWER_ICE].type          = TOWER_ICE;
    types[TOWER_ICE].cost          = 75;
    types[TOWER_ICE].damage        = 5;
    types[TOWER_ICE].range         = 90;
    types[TOWER_ICE].attack_speed  = 1000;
    types[TOWER_ICE].splash        = 0;
    types[TOWER_ICE].slow_pct      = 50;
    types[TOWER_ICE].slow_duration = 2000;
    types[TOWER_ICE].upgrade_cost  = 50;
    types[TOWER_ICE].level         = 1;
    types[TOWER_ICE].name_color    = COLOR_SKY_BLUE;
    { char *n = "\xB1\xF9\xCB\xFE"; memcpy(types[TOWER_ICE].name, n, 5); }
}

int8_t Tower_FindFree(Tower *towers)
{
    uint8_t i;
    for (i = 0; i < MAX_TOWERS; i++) {
        if (!towers[i].active) return (int8_t)i;
    }
    return -1;
}

uint8_t Tower_Build(Tower *towers, uint8_t type, uint16_t col, uint16_t row, TowerType *types)
{
    int8_t id;
    TowerType *tt;

    id = Tower_FindFree(towers);
    if (id < 0) return 0;
    if (col >= GRID_COLS || row >= GRID_ROWS) return 0;

    tt = &types[type];
    towers[id].active         = 1;
    towers[id].type           = type;
    towers[id].level          = 1;
    towers[id].grid_x         = col;
    towers[id].grid_y         = row;
    towers[id].px             = col * CELL_SIZE + CELL_SIZE / 2;
    towers[id].py             = row * CELL_SIZE + CELL_SIZE / 2;
    towers[id].damage         = tt->damage;
    towers[id].range          = tt->range;
    towers[id].attack_speed   = tt->attack_speed;
    towers[id].splash         = tt->splash;
    towers[id].slow_pct       = tt->slow_pct;
    towers[id].slow_duration  = tt->slow_duration;
    towers[id].last_attack_tick = 0;
    towers[id].target_id      = 0xFF;
    towers[id].total_cost     = tt->cost;

    return 1;
}

uint8_t Tower_Upgrade(Tower *t, TowerType *types)
{
    TowerType *tt;
    if (!t->active || t->level >= 3) return 0;

    tt = &types[t->type];
    t->level++;
    t->damage      += tt->damage / 2;
    t->range       += 10;
    t->attack_speed = t->attack_speed > 50 ? t->attack_speed - 50 : t->attack_speed;
    t->total_cost  += tt->upgrade_cost * t->level / 2;

    return 1;
}

void Tower_Sell(Tower *towers, uint8_t id, GameMap *map)
{
    if (id >= MAX_TOWERS || !towers[id].active) return;
    GameMap_RemoveTower(map, towers[id].grid_x, towers[id].grid_y);
    towers[id].active = 0;
}

uint16_t Tower_GetSellValue(Tower *t)
{
    return t->total_cost * 60 / 100;
}

/* Euclidean distance squared (avoid sqrt for performance) */
static uint32_t dist_sq(uint16_t x1, uint16_t y1, int16_t x2, int16_t y2)
{
    int32_t dx = (int32_t)x1 - (int32_t)x2;
    int32_t dy = (int32_t)y1 - (int32_t)y2;
    return (uint32_t)(dx * dx + dy * dy);
}

void Tower_FindTarget(Tower *t, Enemy *enemies)
{
    uint32_t best_dist = 0xFFFFFFFF;
    uint8_t  best_id = 0xFF;
    uint32_t range_sq;
    uint8_t i;

    range_sq = (uint32_t)t->range * t->range;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active || enemies[i].hp <= 0) continue;
        if (enemies[i].spawn_delay > 0) continue;

        uint32_t d = dist_sq(t->px, t->py, (int16_t)enemies[i].x, (int16_t)enemies[i].y);
        if (d <= range_sq) {
            /* Target the enemy furthest along the path (highest waypoint_idx) */
            if (enemies[i].waypoint_idx > (best_id < 0xFF ? enemies[best_id].waypoint_idx : 0)) {
                best_dist = d;
                best_id = i;
            } else if (best_id == 0xFF || (enemies[i].waypoint_idx == enemies[best_id].waypoint_idx && d < best_dist)) {
                best_dist = d;
                best_id = i;
            }
        }
    }

    t->target_id = best_id;
}
