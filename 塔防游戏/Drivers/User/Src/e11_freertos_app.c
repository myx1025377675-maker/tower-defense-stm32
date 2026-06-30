#include "e11_freertos_app.h"

#include "FreeRTOS.h"
#include "task.h"

#include "e08_lab.h"
#include "led.h"
#include "main.h"

#define START_TASK_PRIO 1
#define START_STK_SIZE  256
#define LED_TASK_PRIO   3
#define LED_STK_SIZE    128
#define LCD_TASK_PRIO   2
#define LCD_STK_SIZE    1536

static TaskHandle_t StartTaskHandle;
static TaskHandle_t LedTaskHandle;
static TaskHandle_t LcdTaskHandle;

static void start_task(void *argument);
static void led_task(void *argument);
static void lcd_task(void *argument);

void E11_FreeRTOS_Start(void)
{
    xTaskCreate(start_task, "start_task", START_STK_SIZE, NULL, START_TASK_PRIO, &StartTaskHandle);
    vTaskStartScheduler();

    /* Scheduler returns only when the FreeRTOS heap cannot create idle/timer tasks. */
    for (;;) {
        LED1_Toggle;
        HAL_Delay(100);
    }
}

static void start_task(void *argument)
{
    (void)argument;

    taskENTER_CRITICAL();

    xTaskCreate(led_task, "led_task", LED_STK_SIZE, NULL, LED_TASK_PRIO, &LedTaskHandle);
    xTaskCreate(lcd_task, "lcd_task", LCD_STK_SIZE, NULL, LCD_TASK_PRIO, &LcdTaskHandle);

    taskEXIT_CRITICAL();
    vTaskDelete(StartTaskHandle);
}

static void led_task(void *argument)
{
    (void)argument;

    for (;;) {
        LED1_Toggle;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void lcd_task(void *argument)
{
    (void)argument;

    E08_Init();
    E08_DrawInformationStatic();

    for (;;) {
        uint32_t ms = (uint32_t)((TickType_t)xTaskGetTickCount() * portTICK_PERIOD_MS);
        E08_DecorAnimationTick(ms);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
