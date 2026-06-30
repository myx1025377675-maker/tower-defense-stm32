#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#include <stdint.h>

/* ========== Grid & Map ========== */
#define GRID_COLS     16
#define GRID_ROWS     10
#define CELL_SIZE     40
#define MAP_WIDTH     (GRID_COLS * CELL_SIZE)   /* 640 */
#define MAP_HEIGHT    (GRID_ROWS * CELL_SIZE)   /* 400 */
#define UI_PANEL_X    MAP_WIDTH
#define UI_PANEL_W    160
#define UI_PANEL_Y    0
#define UI_PANEL_H    480
#define INFO_BAR_Y    MAP_HEIGHT
#define INFO_BAR_H    80

/* Grid cell types */
#define CELL_EMPTY    0
#define CELL_PATH     1
#define CELL_TOWER    2
#define CELL_BLOCKED  3

/* ========== Game Limits ========== */
#define MAX_TOWERS        30
#define MAX_ENEMIES       60
#define MAX_PROJECTILES   30
#define MAX_WAYPOINTS     50
#define TOTAL_WAVES       5

/* ========== Tower Types ========== */
#define TOWER_ARROW   0
#define TOWER_CANNON  1
#define TOWER_ICE     2
#define TOWER_TYPE_COUNT 3

/* ========== Enemy Types ========== */
#define ENEMY_NORMAL  0
#define ENEMY_FAST    1
#define ENEMY_TANK    2
#define ENEMY_TYPE_COUNT 3

/* ========== Game States ========== */
#define GAME_STATE_MENU      0
#define GAME_STATE_PLAYING   1
#define GAME_STATE_PAUSED    2
#define GAME_STATE_WAVE_BREAK 3
#define GAME_STATE_OVER      4
#define GAME_STATE_WIN       5

/* ========== UI Button IDs ========== */
#define BTN_START_GAME    0
#define BTN_EXIT           1
#define BTN_RESUME        2
#define BTN_RESTART       3
#define BTN_MAIN_MENU     4
#define BTN_TOWER_ARROW   10
#define BTN_TOWER_CANNON  11
#define BTN_TOWER_ICE     12
#define BTN_UPGRADE       20
#define BTN_SELL          21
#define BTN_CANCEL        22

/* In-game top action buttons */
#define HUD_BTN_Y          (INFO_BAR_Y + 5)
#define HUD_PAUSE_BTN_X    500
#define HUD_EXIT_BTN_X     565
#define HUD_ACTION_BTN_W   58
#define HUD_ACTION_BTN_H   30

/* ========== Colors (32-bit ARGB, as expected by LCD_SetColor/LCD_SetBackColor) ========== */
#define COLOR_BLACK       0xff000000
#define COLOR_WHITE       0xffffffff
#define COLOR_RED         0xffff0000
#define COLOR_GREEN       0xff00ff00
#define COLOR_BLUE        0xff0000ff
#define COLOR_YELLOW      0xffffff00
#define COLOR_CYAN        0xff00ffff
#define COLOR_MAGENTA     0xffff00ff
#define COLOR_GRAY        0xff848284
#define COLOR_DARK_GRAY   0xff424142
#define COLOR_LIGHT_GRAY  0xffc6c3c6
#define COLOR_ORANGE      0xffff8200
#define COLOR_BROWN       0xffa51429
#define COLOR_DARK_GREEN  0xff006500
#define COLOR_LIGHT_GREEN 0xff8cff00
#define COLOR_GOLD        0xffffeb00
#define COLOR_PURPLE      0xff8400ff
#define COLOR_SKY_BLUE    0xff84cfef
#define COLOR_PINK        0xffff82c6

/* Warm hand-painted UI palette */
#define COLOR_CREAM       0xfffff1d6
#define COLOR_PARCHMENT   0xffffe3ad
#define COLOR_SAND        0xffe8c47f
#define COLOR_HONEY       0xffffc857
#define COLOR_AMBER       0xfff39c3d
#define COLOR_CLAY        0xffc86f3d
#define COLOR_TERRACOTTA  0xffa94d32
#define COLOR_WOOD        0xff8b5a2b
#define COLOR_DARK_WOOD   0xff5c3a21
#define COLOR_MOSS        0xff8fb56b
#define COLOR_GRASS_SOFT  0xffbddf8a
#define COLOR_LEAF        0xff5f8f48
#define COLOR_STONE_SOFT  0xffb7a990
#define COLOR_INK_BROWN   0xff3f2616
#define COLOR_ROSE_DUST   0xffd98c74

/* ========== Game Parameters ========== */
#define INITIAL_GOLD      200
#define INITIAL_LIVES     20
#define MAX_LIVES         99

/* ========== Projectile Types ========== */
#define PROJ_ARROW   0
#define PROJ_CANNON  1
#define PROJ_ICE     2

/* ========== Tower Data Structures ========== */
typedef struct {
    uint8_t  type;          /* TOWER_ARROW / TOWER_CANNON / TOWER_ICE */
    uint16_t cost;
    uint16_t damage;
    uint16_t range;         /* in pixels */
    uint16_t attack_speed;  /* cooldown in ms */
    uint16_t splash;        /* splash radius in pixels (0 = single target) */
    uint16_t slow_pct;      /* slow percentage (0 = no slow) */
    uint16_t slow_duration; /* slow duration in ms */
    uint16_t upgrade_cost;
    uint8_t  level;
    char     name[12];
    uint32_t name_color;
} TowerType;

typedef struct {
    uint8_t  active;
    uint8_t  type;
    uint8_t  level;
    uint16_t grid_x;
    uint16_t grid_y;
    uint16_t px;
    uint16_t py;
    uint32_t last_attack_tick;
    uint8_t  target_id;
    uint16_t damage;
    uint16_t range;
    uint16_t attack_speed;
    uint16_t splash;
    uint16_t slow_pct;
    uint16_t slow_duration;
    uint16_t total_cost;    /* sum invested (build + upgrades) */
} Tower;

typedef struct {
    uint8_t  active;
    uint8_t  type;
    int16_t  x, y;          /* pixel position */
    uint8_t  target_id;     /* target enemy */
    uint8_t  tower_type;    /* source tower type */
    uint16_t damage;
    uint16_t splash;
    uint16_t slow_pct;
    uint16_t slow_duration;
    uint16_t speed;         /* pixels per tick */
    uint32_t color;
} Projectile;

/* ========== Enemy Data Structures ========== */
typedef struct {
    uint8_t  type;
    uint16_t max_hp;
    uint16_t speed;         /* pixels per second */
    uint16_t reward;
    uint32_t color;
    char     name[8];
} EnemyType;

typedef struct {
    uint8_t  active;
    uint8_t  type;
    uint8_t  waypoint_idx;  /* current target waypoint */
    float    x, y;          /* pixel position (float for smooth movement) */
    uint16_t max_hp;
    int16_t  hp;
    uint16_t speed;
    uint16_t base_speed;
    uint16_t reward;
    uint32_t color;
    uint32_t slow_until;    /* tick when slow effect expires */
    uint32_t spawn_delay;   /* ticks until spawning at waypoint 0 */
} Enemy;

/* ========== Wave Definition ========== */
typedef struct {
    uint8_t  enemy_type;
    uint8_t  count;
    uint16_t spawn_interval; /* ms between spawns */
} WaveEntry;

typedef struct {
    WaveEntry entries[8];
    uint8_t   entry_count;
    uint16_t  pre_delay;     /* ms before wave starts */
} Wave;

/* ========== Map Data ========== */
typedef struct {
    uint8_t grid[GRID_ROWS][GRID_COLS];
    uint16_t waypoint_x[MAX_WAYPOINTS];
    uint16_t waypoint_y[MAX_WAYPOINTS];
    uint8_t  waypoint_count;
} GameMap;

/* ========== Game State ========== */
typedef struct {
    uint8_t  state;          /* GAME_STATE_* */
    int32_t  gold;
    uint8_t  lives;
    uint32_t score;
    uint8_t  current_wave;
    uint8_t  wave_enemy_index;
    uint8_t  wave_entry_index;
    uint16_t wave_spawn_timer;
    uint16_t wave_pre_delay_timer;
    uint8_t  wave_active;
    uint8_t  all_waves_done;
    uint8_t  selected_tower_type;
    uint8_t  selected_tower_id;  /* index into towers[] for upgrade/sell */
    uint32_t game_tick_ms;
    Tower    towers[MAX_TOWERS];
    Enemy    enemies[MAX_ENEMIES];
    Projectile projectiles[MAX_PROJECTILES];
    GameMap  map;
    TowerType tower_types[TOWER_TYPE_COUNT];
    EnemyType enemy_types[ENEMY_TYPE_COUNT];
    Wave     waves[TOTAL_WAVES];
} GameState;

/* ========== Touch / UI Event ========== */
typedef struct {
    uint8_t  event_type;  /* 0=none, 1=tap, 2=button */
    uint16_t x, y;
    uint16_t btn_id;
} UIEvent;

/* ========== Message Types for Queues ========== */
#define MSG_TOUCH_EVENT  1
#define MSG_GAME_STATE   2

typedef struct {
    uint8_t type;
    UIEvent event;
} GameMessage;

/* Global game state */
extern GameState g_game;

#endif /* GAME_DEFS_H */
