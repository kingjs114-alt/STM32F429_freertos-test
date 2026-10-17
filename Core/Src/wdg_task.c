/*
 * wdg_task.c - 독립 워치독(IWDG) 감시 Task
 *
 * 단순히 "주기적으로 IWDG refresh" 만 하면, 다른 Task 가 멈춰도 워치독은
 * 계속 refresh 되어 리셋이 안 된다. 그래서
 *   - 감시 대상 Task 들이 각자 alive 비트를 Event Group 에 set 하고
 *   - WdgTask 는 주기마다 "모든 비트가 set 되었는지" 확인한 뒤에만 refresh 한다.
 * 하나라도 멈추면 refresh 가 끊기고, 약 4초 후 MCU 가 리셋된다.
 */
#include "main.h"
#include "app.h"

/* 감시 주기. LED(1s), LCD(200ms) 보고 주기보다 길어야 한다. */
#define WDG_CHECK_PERIOD_MS     1500U

/*
 * IWDG 타임아웃 = (Prescaler / LSI) x (Reload + 1)
 *              = (128 / 32kHz) x 1000 = 약 4초
 * LSI 는 17~47kHz 로 오차가 커서(최악 약 2.7초) 감시 주기보다 넉넉히 잡는다.
 */
#define WDG_PRESCALER           IWDG_PRESCALER_128
#define WDG_RELOAD              999U

static IWDG_HandleTypeDef hiwdg;
volatile uint32_t g_wdgRefreshCount;

void Watchdog_Init(void)
{
    /* 디버거로 정지(break)했을 때 워치독도 멈추게 함 -> 디버깅 중 리셋 방지 */
    __HAL_DBGMCU_FREEZE_IWDG();

    hiwdg.Instance       = IWDG;
    hiwdg.Init.Prescaler = WDG_PRESCALER;
    hiwdg.Init.Reload    = WDG_RELOAD;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {   /* 한 번 시작하면 끌 수 없음 */
        Error_Handler();
    }
}

void WdgTask(void *argument)
{
    (void)argument;

    Watchdog_Init();

    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(WDG_CHECK_PERIOD_MS));

        /* 비트를 읽으면서 동시에 clear (반환값 = clear 직전 값) */
        EventBits_t alive = xEventGroupClearBits(g_aliveEvents, ALIVE_BITS_ALL);

        if ((alive & ALIVE_BITS_ALL) == ALIVE_BITS_ALL) {
            HAL_IWDG_Refresh(&hiwdg);
            g_wdgRefreshCount++;
        }
        /* else: 누군가 응답 없음 -> refresh 안 함 -> 곧 리셋 */
    }
}
