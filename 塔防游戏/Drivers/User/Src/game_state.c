#include "game_state.h"
#include "game_map.h"
#include "game_tower.h"
#include "game_enemy.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Global game state */
GameState g_game;

void Waves_Init(Wave *waves)
{
    uint8_t w;

    for (w = 0; w < TOTAL_WAVES; w++)
        memset(&waves[w], 0, sizeof(Wave));

    /* Wave 1: 5 Normal enemies */
    waves[0].entry_count = 1;
    waves[0].entries[0].enemy_type = ENEMY_NORMAL;
    waves[0].entries[0].count = 5;
    waves[0].entries[0].spawn_interval = 800;
    waves[0].pre_delay = 2000;

    /* Wave 2: 3 Normal + 3 Fast */
    waves[1].entry_count = 2;
    waves[1].entries[0].enemy_type = ENEMY_NORMAL;
    waves[1].entries[0].count = 3;
    waves[1].entries[0].spawn_interval = 700;
    waves[1].entries[1].enemy_type = ENEMY_FAST;
    waves[1].entries[1].count = 3;
    waves[1].entries[1].spawn_interval = 600;
    waves[1].pre_delay = 1500;

    /* Wave 3: 5 Normal + 5 Fast */
    waves[2].entry_count = 2;
    waves[2].entries[0].enemy_type = ENEMY_NORMAL;
    waves[2].entries[0].count = 5;
    waves[2].entries[0].spawn_interval = 600;
    waves[2].entries[1].enemy_type = ENEMY_FAST;
    waves[2].entries[1].count = 5;
    waves[2].entries[1].spawn_interval = 500;
    waves[2].pre_delay = 1500;

    /* Wave 4: 5 Normal + 3 Fast + 2 Tank */
    waves[3].entry_count = 3;
    waves[3].entries[0].enemy_type = ENEMY_NORMAL;
    waves[3].entries[0].count = 5;
    waves[3].entries[0].spawn_interval = 500;
    waves[3].entries[1].enemy_type = ENEMY_FAST;
    waves[3].entries[1].count = 3;
    waves[3].entries[1].spawn_interval = 400;
    waves[3].entries[2].enemy_type = ENEMY_TANK;
    waves[3].entries[2].count = 2;
    waves[3].entries[2].spawn_interval = 1200;
    waves[3].pre_delay = 1500;

    /* Wave 5: 5 Fast + 5 Tank (final boss wave) */
    waves[4].entry_count = 2;
    waves[4].entries[0].enemy_type = ENEMY_FAST;
    waves[4].entries[0].count = 5;
    waves[4].entries[0].spawn_interval = 400;
    waves[4].entries[1].enemy_type = ENEMY_TANK;
    waves[4].entries[1].count = 5;
    waves[4].entries[1].spawn_interval = 1000;
    waves[4].pre_delay = 2000;
}

void GameState_Init(void)
{
    memset(&g_game, 0, sizeof(g_game));
    GameMap_Init(&g_game.map);
    TowerTypes_Init(g_game.tower_types);
    EnemyTypes_Init(g_game.enemy_types);
    Waves_Init(g_game.waves);
    g_game.state = GAME_STATE_MENU;
}

void GameState_Reset(void)
{
    memset(g_game.towers, 0, sizeof(g_game.towers));
    memset(g_game.enemies, 0, sizeof(g_game.enemies));
    memset(g_game.projectiles, 0, sizeof(g_game.projectiles));
    GameMap_Init(&g_game.map);
    g_game.gold = INITIAL_GOLD;
    g_game.lives = INITIAL_LIVES;
    g_game.score = 0;
    g_game.current_wave = 0;
    g_game.wave_enemy_index = 0;
    g_game.wave_entry_index = 0;
    g_game.wave_spawn_timer = 0;
    g_game.wave_pre_delay_timer = 0;
    g_game.wave_active = 0;
    g_game.all_waves_done = 0;
    g_game.selected_tower_type = 0xFF;
    g_game.selected_tower_id = 0xFF;
    g_game.game_tick_ms = 0;
}

void GameState_StartWave(void)
{
    if (g_game.current_wave >= TOTAL_WAVES) {
        g_game.all_waves_done = 1;
        return;
    }
    g_game.wave_active = 1;
    g_game.wave_entry_index = 0;
    g_game.wave_enemy_index = 0;
    g_game.wave_spawn_timer = 0;
    g_game.wave_pre_delay_timer = g_game.waves[g_game.current_wave].pre_delay;

    printf("\r\n=== Wave %d/%d ===\r\n", g_game.current_wave + 1, TOTAL_WAVES);
}

void GameState_Hurt(uint8_t damage)
{
    if (g_game.lives <= damage) {
        g_game.lives = 0;
    } else {
        g_game.lives -= damage;
    }
}

/* Forward declaration for projectile helper */
static void SpawnProjectile(Tower *t, Enemy *target);

static void ProcessTowers(uint32_t now_ms)
{
    uint8_t i;
    for (i = 0; i < MAX_TOWERS; i++) {
        Tower *t = &g_game.towers[i];
        if (!t->active) continue;

        Tower_FindTarget(t, g_game.enemies);

        if (t->target_id < MAX_ENEMIES) {
            Enemy *e = &g_game.enemies[t->target_id];
            if (e->active && e->hp > 0 && (now_ms - t->last_attack_tick) >= t->attack_speed) {
                SpawnProjectile(t, e);
                t->last_attack_tick = now_ms;
            }
        }
    }
}

static void SpawnProjectile(Tower *t, Enemy *target)
{
    uint8_t i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!g_game.projectiles[i].active) break;
    }
    if (i >= MAX_PROJECTILES) return;

    Projectile *p = &g_game.projectiles[i];
    p->active = 1;
    p->type = t->type;
    p->x = (int16_t)t->px;
    p->y = (int16_t)t->py;
    p->target_id = t->target_id;
    p->damage = t->damage;
    p->splash = t->splash;
    p->slow_pct = t->slow_pct;
    p->slow_duration = t->slow_duration;
    p->speed = 400;
    p->color = t->splash > 0 ? COLOR_ORANGE : (t->slow_pct > 0 ? COLOR_SKY_BLUE : COLOR_YELLOW);
}

static void ProcessProjectiles(uint32_t now_ms, uint32_t dt_ms)
{
    uint8_t i;
    float dt_s = (float)dt_ms / 1000.0f;

    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &g_game.projectiles[i];
        if (!p->active) continue;

        /* Target still valid? */
        if (p->target_id >= MAX_ENEMIES ||
            !g_game.enemies[p->target_id].active ||
            g_game.enemies[p->target_id].hp <= 0) {
            p->active = 0;
            continue;
        }

        Enemy *e = &g_game.enemies[p->target_id];
        float dx = e->x - (float)p->x;
        float dy = e->y - (float)p->y;
        float dist = sqrtf(dx * dx + dy * dy);

        float step = p->speed * dt_s;
        if (step >= dist || dist < 5.0f) {
            /* Hit! */
            if (p->splash > 0) {
                /* Splash damage to all enemies in radius */
                uint8_t j;
                for (j = 0; j < MAX_ENEMIES; j++) {
                    Enemy *ej = &g_game.enemies[j];
                    if (!ej->active || ej->hp <= 0) continue;
                    if (ej->spawn_delay > 0) continue;
                    float sx = ej->x - e->x;
                    float sy = ej->y - e->y;
                    if (sqrtf(sx * sx + sy * sy) <= p->splash) {
                        ej->hp -= (int16_t)p->damage;
                    }
                }
            } else {
                e->hp -= (int16_t)p->damage;
            }

            /* Apply slow */
            if (p->slow_pct > 0) {
                Enemy_ApplySlow(e, p->slow_pct, p->slow_duration, now_ms);
            }

            p->active = 0;
        } else {
            p->x += (int16_t)(dx / dist * step);
            p->y += (int16_t)(dy / dist * step);
        }
    }
}

static void ProcessEnemies(uint32_t now_ms, uint32_t dt_ms)
{
    uint8_t i;

    Enemy_Move(g_game.enemies, &g_game.map, dt_ms);

    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &g_game.enemies[i];
        if (!e->active) continue;

        /* Clear expired slow */
        if (e->slow_until > 0 && now_ms >= e->slow_until) {
            e->speed = e->base_speed;
            e->slow_until = 0;
        }

        /* Enemy killed */
        if (e->hp <= 0 && e->waypoint_idx < g_game.map.waypoint_count - 1) {
            g_game.gold += e->reward;
            g_game.score += e->reward * 10;
            e->active = 0;
        }

        /* Enemy reached end */
        if (e->waypoint_idx >= g_game.map.waypoint_count - 1) {
            GameState_Hurt(1);
            g_game.score -= 5;
            e->active = 0;
        }
    }
}

static void ProcessWave(uint32_t dt_ms)
{
    Wave *w;

    if (!g_game.wave_active) return;
    if (g_game.wave_pre_delay_timer > 0) {
        if (g_game.wave_pre_delay_timer <= dt_ms) {
            g_game.wave_pre_delay_timer = 0;
        } else {
            g_game.wave_pre_delay_timer -= dt_ms;
            return;
        }
    }

    w = &g_game.waves[g_game.current_wave];
    if (g_game.wave_entry_index >= w->entry_count) {
        /* All entries spawned, check if wave is done */
        if (Enemy_CountActive(g_game.enemies) == 0 && Enemy_CountPending(g_game.enemies) == 0) {
            g_game.wave_active = 0;
            g_game.current_wave++;
            if (g_game.current_wave >= TOTAL_WAVES) {
                g_game.all_waves_done = 1;
            } else {
                g_game.state = GAME_STATE_WAVE_BREAK;
                g_game.gold += 50;
            }
        }
        return;
    }

    WaveEntry *entry = &w->entries[g_game.wave_entry_index];
    g_game.wave_spawn_timer += dt_ms;

    if (g_game.wave_spawn_timer >= entry->spawn_interval) {
        g_game.wave_spawn_timer = 0;
        Enemy_Spawn(g_game.enemies, entry->enemy_type,
                    (uint16_t)(g_game.wave_enemy_index * 200), /* stagger spawns */
                    &g_game.map, g_game.enemy_types);
        g_game.wave_enemy_index++;

        if (g_game.wave_enemy_index >= entry->count) {
            g_game.wave_entry_index++;
            g_game.wave_enemy_index = 0;
            g_game.wave_spawn_timer = 0;
        }
    }
}

static void CheckWinLose(void)
{
    if (g_game.lives == 0) {
        g_game.state = GAME_STATE_OVER;
        return;
    }

    if (g_game.all_waves_done &&
        !g_game.wave_active &&
        Enemy_CountActive(g_game.enemies) == 0 &&
        Enemy_CountPending(g_game.enemies) == 0) {
        g_game.state = GAME_STATE_WIN;
    }
}

void GameState_Update(uint32_t dt_ms)
{
    uint32_t now_ms;

    if (g_game.state != GAME_STATE_PLAYING) return;

    g_game.game_tick_ms += dt_ms;
    now_ms = g_game.game_tick_ms;

    ProcessWave(dt_ms);
    ProcessEnemies(now_ms, dt_ms);
    ProcessTowers(now_ms);
    ProcessProjectiles(now_ms, dt_ms);
    CheckWinLose();
}
