#include "game_enemy.h"
#include <string.h>
#include <math.h>

void EnemyTypes_Init(EnemyType *types)
{
    /* Normal */
    types[ENEMY_NORMAL].type  = ENEMY_NORMAL;
    types[ENEMY_NORMAL].max_hp = 100;
    types[ENEMY_NORMAL].speed = 60;
    types[ENEMY_NORMAL].reward = 20;
    types[ENEMY_NORMAL].color = COLOR_RED;
    { char *n = "Normal"; memcpy(types[ENEMY_NORMAL].name, n, 7); }

    /* Fast */
    types[ENEMY_FAST].type   = ENEMY_FAST;
    types[ENEMY_FAST].max_hp = 50;
    types[ENEMY_FAST].speed  = 120;
    types[ENEMY_FAST].reward = 25;
    types[ENEMY_FAST].color  = COLOR_YELLOW;
    { char *n = "Fast"; memcpy(types[ENEMY_FAST].name, n, 5); }

    /* Tank */
    types[ENEMY_TANK].type   = ENEMY_TANK;
    types[ENEMY_TANK].max_hp = 250;
    types[ENEMY_TANK].speed  = 35;
    types[ENEMY_TANK].reward = 40;
    types[ENEMY_TANK].color  = COLOR_PURPLE;
    { char *n = "Tank"; memcpy(types[ENEMY_TANK].name, n, 5); }
}

uint8_t Enemy_Spawn(Enemy *enemies, uint8_t type, uint16_t spawn_delay, GameMap *map, EnemyType *types)
{
    uint8_t i;
    EnemyType *et;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) break;
    }
    if (i >= MAX_ENEMIES) return 0;

    et = &types[type];
    enemies[i].active        = 1;
    enemies[i].type          = type;
    enemies[i].waypoint_idx  = 0;
    enemies[i].x             = (float)map->waypoint_x[0];
    enemies[i].y             = (float)map->waypoint_y[0];
    enemies[i].max_hp        = et->max_hp;
    enemies[i].hp            = (int16_t)et->max_hp;
    enemies[i].speed         = et->speed;
    enemies[i].base_speed    = et->speed;
    enemies[i].reward        = et->reward;
    enemies[i].color         = et->color;
    enemies[i].slow_until    = 0;
    enemies[i].spawn_delay   = spawn_delay;

    return 1;
}

static float lerp_between(float a, float b, float t)
{
    return a + (b - a) * t;
}

void Enemy_Move(Enemy *enemies, GameMap *map, uint32_t dt_ms)
{
    uint8_t i;
    float dt_s = (float)dt_ms / 1000.0f;

    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->active || e->hp <= 0) continue;

        if (e->spawn_delay > 0) {
            if (e->spawn_delay <= dt_ms) {
                e->spawn_delay = 0;
            } else {
                e->spawn_delay -= dt_ms;
                continue;
            }
        }

        /* Move toward next waypoint */
        uint8_t wp = e->waypoint_idx;
        if (wp >= map->waypoint_count - 1) {
            e->hp = -1;
            continue;
        }

        float tx = (float)map->waypoint_x[wp + 1];
        float ty = (float)map->waypoint_y[wp + 1];
        float dx = tx - e->x;
        float dy = ty - e->y;
        float dist = sqrtf(dx * dx + dy * dy);

        float step = e->speed * dt_s;
        if (step >= dist) {
            e->x = tx;
            e->y = ty;
            e->waypoint_idx++;
        } else {
            e->x += dx / dist * step;
            e->y += dy / dist * step;
        }
    }
}

uint8_t Enemy_ReachedEnd(Enemy *e, GameMap *map)
{
    if (!e->active) return 0;
    return (e->waypoint_idx >= map->waypoint_count - 1 && e->hp <= 0);
}

void Enemy_ApplySlow(Enemy *e, uint16_t pct, uint16_t duration_ms, uint32_t now_ms)
{
    if (!e->active) return;
    e->slow_until = now_ms + duration_ms;
    e->speed = (uint16_t)((uint32_t)e->base_speed * (100 - pct) / 100);
    if (e->speed < 10) e->speed = 10;
}

uint8_t Enemy_CountActive(Enemy *enemies)
{
    uint8_t i, cnt = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && enemies[i].hp > 0) cnt++;
    }
    return cnt;
}

uint8_t Enemy_CountPending(Enemy *enemies)
{
    uint8_t i, cnt = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && enemies[i].hp > 0 && enemies[i].spawn_delay > 0) cnt++;
    }
    return cnt;
}
