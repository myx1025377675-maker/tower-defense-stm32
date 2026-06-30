#include "game_map.h"
#include <string.h>

/*
 * Cell-by-cell waypoints define the exact enemy path.
 * Each step moves exactly 1 grid cell along the path.
 * Path: start at (0,4), wind through the map, exit at (15,4).
 *
 * NEW PATH (redesigned — every cell verified):
 *   (0,4) right → (3,4), up → (3,1), right → (7,1),
 *   down → (7,7), right → (11,7), up → (11,3),
 *   right → (15,3), final step → (15,4).
 */
static const int8_t path_xy[][2] = {
    /* Segment 1: right along row 4 */
    { 0, 4}, { 1, 4}, { 2, 4}, { 3, 4},
    /* Segment 2: up along col 3 */
    { 3, 3}, { 3, 2}, { 3, 1},
    /* Segment 3: right along row 1 */
    { 4, 1}, { 5, 1}, { 6, 1}, { 7, 1},
    /* Segment 4: down along col 7 */
    { 7, 2}, { 7, 3}, { 7, 4}, { 7, 5}, { 7, 6}, { 7, 7},
    /* Segment 5: right along row 7 */
    { 8, 7}, { 9, 7}, {10, 7}, {11, 7},
    /* Segment 6: up along col 11 */
    {11, 6}, {11, 5}, {11, 4}, {11, 3},
    /* Segment 7: right along row 3 */
    {12, 3}, {13, 3}, {14, 3}, {15, 3},
    /* Segment 8: final step down to exit */
    {15, 4},
    /* sentinel */
    {-1, -1}
};

#define PATH_STEP_COUNT (sizeof(path_xy) / sizeof(path_xy[0]))

void GameMap_Init(GameMap *map)
{
    uint8_t i, w, r, c;

    memset(map->grid, CELL_EMPTY, sizeof(map->grid));

    /* Count waypoints and set pixel coordinates */
    w = 0;
    while (w < PATH_STEP_COUNT - 1 && w < MAX_WAYPOINTS &&
           path_xy[w][0] >= 0 && path_xy[w][1] >= 0) {
        w++;
    }
    map->waypoint_count = w;

    for (i = 0; i < w; i++) {
        map->waypoint_x[i] = (uint16_t)((uint8_t)path_xy[i][0]) * CELL_SIZE + CELL_SIZE / 2;
        map->waypoint_y[i] = (uint16_t)((uint8_t)path_xy[i][1]) * CELL_SIZE + CELL_SIZE / 2;
        /* Mark path cell */
        map->grid[(uint8_t)path_xy[i][1]][(uint8_t)path_xy[i][0]] = CELL_PATH;
    }

    /* Block some cells for obstacles (avoid overwriting path cells) */
    {
        uint8_t j;
        static const uint8_t block_r[] = {0, 5, 9, 13, 1, 6, 10, 14, 2, 8};
        static const uint8_t block_c[] = {0, 3, 1,  5, 7, 5,  2,  7, 8, 4};
        for (j = 0; j < 10; j++) {
            r = block_r[j]; c = block_c[j];
            if (map->grid[r][c] == CELL_EMPTY)
                map->grid[r][c] = CELL_BLOCKED;
        }
    }
}

uint8_t GameMap_CanPlace(GameMap *map, uint16_t col, uint16_t row)
{
    if (col >= GRID_COLS || row >= GRID_ROWS) return 0;
    return map->grid[row][col] == CELL_EMPTY;
}

void GameMap_PlaceTower(GameMap *map, uint16_t col, uint16_t row)
{
    if (col < GRID_COLS && row < GRID_ROWS)
        map->grid[row][col] = CELL_TOWER;
}

void GameMap_RemoveTower(GameMap *map, uint16_t col, uint16_t row)
{
    if (col < GRID_COLS && row < GRID_ROWS)
        map->grid[row][col] = CELL_EMPTY;
}
