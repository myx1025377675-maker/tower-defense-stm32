#include "game_tasks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "game_defs.h"
#include "game_state.h"
#include "game_render.h"
#include "game_ui.h"
#include "game_map.h"
#include "game_tower.h"
#include "lcd_rgb.h"
#include "touch_800x480.h"
#include "led.h"

/* Task priorities */
#define INPUT_TASK_PRIO     2
#define LOGIC_TASK_PRIO     1
#define RENDER_TASK_PRIO    1

/* Stack sizes */
#define INPUT_STK_SIZE      512
#define LOGIC_STK_SIZE      1536
#define RENDER_STK_SIZE     2048

/* Timing */
#define INPUT_PERIOD_MS     20
#define LOGIC_PERIOD_MS     33    /* ~30 updates per second */
#define RENDER_PERIOD_MS    50    /* ~20 fps (reduced for performance) */

/* Queue */
#define TOUCH_QUEUE_LEN     10

static TaskHandle_t InputTaskHandle;
static TaskHandle_t LogicTaskHandle;
static TaskHandle_t RenderTaskHandle;
static QueueHandle_t TouchQueue;
static SemaphoreHandle_t GameMutex;

static void input_task(void *arg);
static void logic_task(void *arg);
static void render_task(void *arg);

void GameTasks_Start(void)
{
    TouchQueue = xQueueCreate(TOUCH_QUEUE_LEN, sizeof(UIEvent));
    GameMutex  = xSemaphoreCreateMutex();

    xTaskCreate(input_task,  "input_task",  INPUT_STK_SIZE,  NULL, INPUT_TASK_PRIO,  &InputTaskHandle);
    xTaskCreate(logic_task,  "logic_task",  LOGIC_STK_SIZE,  NULL, LOGIC_TASK_PRIO,  &LogicTaskHandle);
    xTaskCreate(render_task, "render_task", RENDER_STK_SIZE, NULL, RENDER_TASK_PRIO, &RenderTaskHandle);

    vTaskStartScheduler();

    for (;;) {
        LED1_Toggle;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void input_task(void *arg)
{
    (void)arg;
    UIEvent evt;
    uint8_t was_touched = 0;
    TickType_t last_wake;

    last_wake = xTaskGetTickCount();

    for (;;) {
        Touch_Scan();

        if (touchInfo.flag == 1) {
            if (!was_touched) {
                /* New touch */
                evt.event_type = 1;
                evt.x = touchInfo.x[0];
                evt.y = touchInfo.y[0];
                evt.btn_id = 0;
                xQueueSend(TouchQueue, &evt, 0);
                was_touched = 1;
            }
        } else {
            was_touched = 0;
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(INPUT_PERIOD_MS));
    }
}

static void logic_task(void *arg)
{
    (void)arg;
    UIEvent evt;
    TickType_t last_wake;
    uint32_t dt_ms = LOGIC_PERIOD_MS;

    last_wake = xTaskGetTickCount();

    for (;;) {
        /* Process any pending touch events */
        while (xQueueReceive(TouchQueue, &evt, 0) == pdTRUE) {
            xSemaphoreTake(GameMutex, portMAX_DELAY);
            GameUI_ProcessInput(&evt);
            xSemaphoreGive(GameMutex);
        }

        /* Update game state */
        xSemaphoreTake(GameMutex, portMAX_DELAY);
        if (g_game.state == GAME_STATE_PLAYING) {
            GameState_Update(dt_ms);
        }
        xSemaphoreGive(GameMutex);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(LOGIC_PERIOD_MS));
    }
}

static void render_task(void *arg)
{
    (void)arg;
    TickType_t last_wake;
    uint8_t last_state = 0xFF;
    uint8_t need_full_redraw = 1;

    last_wake = xTaskGetTickCount();

    for (;;) {
        xSemaphoreTake(GameMutex, portMAX_DELAY);

        uint8_t cur_state = g_game.state;

        if (cur_state != last_state) {
            need_full_redraw = 1;
            last_state = cur_state;
        }

        switch (cur_state) {
        case GAME_STATE_MENU:
            if (need_full_redraw) {
                GameUI_DrawMenu();
                need_full_redraw = 0;
            }
            break;

        case GAME_STATE_PLAYING:
        case GAME_STATE_PAUSED:
            GameRender_All();
            break;

        case GAME_STATE_WAVE_BREAK:
            GameRender_All();
            GameUI_DrawWaveBreak();
            break;

        case GAME_STATE_OVER:
            if (need_full_redraw) {
                GameUI_DrawGameOver();
                need_full_redraw = 0;
            }
            break;

        case GAME_STATE_WIN:
            if (need_full_redraw) {
                GameUI_DrawWin();
                need_full_redraw = 0;
            }
            break;

        default:
            break;
        }

        xSemaphoreGive(GameMutex);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(RENDER_PERIOD_MS));
    }
}
