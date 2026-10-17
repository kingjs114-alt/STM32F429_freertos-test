/*
 * led_task.c - 1초 간격 LED 깜박임
 *
 * 1ms 타이머 인터럽트가 1000번 카운트할 때마다 Task Notification 으로
 * 이 Task 를 깨운다. 깨어나기 전까지 이 Task 는 Blocked 상태라 CPU 를 쓰지 않는다.
 *
 * (참고) 타이머 인터럽트 없이 Task 만으로 하려면:
 *     vTaskDelayUntil(&last, pdMS_TO_TICKS(1000));
 */
#include "app.h"
#include "stm32f429i_discovery.h"

volatile uint32_t g_ledToggleCount;

void LedTask(void *argument)
{
    (void)argument;

    for (;;) {
        /* 1초 신호를 기다린다. 1.5초 안에 안 오면 타이머 이상 -> alive 보고 안 함 */
        uint32_t notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1500));
        if (notified == 0) {
            continue;
        }

        BSP_LED_Toggle(LED3);
        g_ledToggleCount++;

        /* 워치독 감시자에게 "나 살아있음" 보고 */
        xEventGroupSetBits(g_aliveEvents, ALIVE_BIT_LED);
    }
}
