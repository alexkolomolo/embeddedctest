
#include <stdio.h>
#include <pthread.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "console.h"

#define mainQUEUE_R_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_S_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define mainQUEUE_L (2)

#define mainVALUE_TASK (300UL)
#define mainVALUE_TIMER (400UL)

static void prvQueueRTask(void *pvParameters);
static void prvQueueSTask(void *pvParameters);

static void prvQueueSTimerCallback(TimerHandle_t xTimerHandle);

static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;

void main_console(void)
{
    const TickType_t xTimerPeriod = pdMS_TO_TICKS(2000UL);

    xQueue = xQueueCreate(mainQUEUE_L, sizeof(uint32_t));

    if (xQueue != NULL)
    {
        xTaskCreate(prvQueueRTask,
                    "TaskR",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    mainQUEUE_R_TASK_PRIORITY,
                    NULL);

        xTaskCreate(prvQueueSTask,
                    "TaskS",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    mainQUEUE_S_TASK_PRIORITY,
                    NULL);

        xTimer = xTimerCreate("Timer",
                              xTimerPeriod,
                              pdTRUE,
                              NULL,
                              prvQueueSTimerCallback);

        if (xTimer != NULL)
        {
            xTimerStart(xTimer, 0);
        }

        vTaskStartScheduler();
    }

    for (;;)
    {
    }
}

static void prvQueueSTask(void *pvParameters)
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = pdMS_TO_TICKS(1000UL);
    const uint32_t ulSValue = mainVALUE_TASK;

    (void)pvParameters;

    xNextWakeTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&xNextWakeTime, xBlockTime);

        xQueueSend(xQueue, &ulSValue, 0U);
    }
}

static void prvQueueSTimerCallback(TimerHandle_t xTimerHandle)
{
    const uint32_t ulSValue = mainVALUE_TIMER;

    (void)xTimerHandle;

    xQueueSend(xQueue, &ulSValue, 0U);
}

static void prvQueueRTask(void *pvParameters)
{
    uint32_t ulRValue;

    (void)pvParameters;

    for (;;)
    {
        xQueueReceive(xQueue, &ulRValue, portMAX_DELAY);

        if (ulRValue == mainVALUE_TASK)
        {
            console_print("Task Value = %d\n", ulRValue);
        }
        else if (ulRValue == mainVALUE_TIMER)
        {
            console_print("Timer Value = %d\n", ulRValue);
        }
    }
}
