/*
 * app_timer.c - 1ms 타이머 인터럽트에서 하는 일
 *
 * ISR(인터럽트) 규칙
 *   1. 짧게! 오래 걸리는 일은 Task 에게 넘긴다.
 *   2. FreeRTOS API 는 반드시 "...FromISR" 버전만 사용한다.
 *   3. Task 를 깨웠다면 portYIELD_FROM_ISR() 로 즉시 문맥 전환을 요청한다.
 */
#include "app.h"

#define LED_PERIOD_MS   1000U

volatile uint32_t g_msCounter;

void App_Timer1msISR(void)
{
    static uint32_t ledDivider = 0;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    g_msCounter++;

    /* 1000ms 마다 LedTask 에게 "토글해!" 알림 (Task Notification) */
    if (++ledDivider >= LED_PERIOD_MS) {
        ledDivider = 0;
        if (g_ledTaskHandle != NULL) {
            vTaskNotifyGiveFromISR(g_ledTaskHandle, &higherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
