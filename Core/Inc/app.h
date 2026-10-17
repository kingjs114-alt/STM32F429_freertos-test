/*
 * app.h - 애플리케이션 Task 공용 정의
 */
#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/*------------------------------------------------------------------------
 * Task 우선순위 (숫자가 클수록 높음, 0 은 Idle Task)
 *----------------------------------------------------------------------*/
#define WDG_TASK_PRIO       (tskIDLE_PRIORITY + 3)  /* 감시자는 가장 높게 */
#define LED_TASK_PRIO       (tskIDLE_PRIORITY + 2)  /* 1초 주기, 처리 시간 짧음 */
#define LCD_TASK_PRIO       (tskIDLE_PRIORITY + 1)  /* 화면 그리기는 오래 걸리므로 낮게 */

/* Task 스택 크기 (단위: word = 4byte) */
#define WDG_TASK_STACK      256
#define LED_TASK_STACK      256
#define LCD_TASK_STACK      768                     /* snprintf + LCD 드라이버 사용 */

/*------------------------------------------------------------------------
 * Watchdog 감시용 "살아있음" 비트
 * 각 Task 는 정상적으로 한 바퀴 돌 때마다 자기 비트를 set 한다.
 *----------------------------------------------------------------------*/
#define ALIVE_BIT_LED       (1U << 0)
#define ALIVE_BIT_LCD       (1U << 1)
#define ALIVE_BITS_ALL      (ALIVE_BIT_LED | ALIVE_BIT_LCD)

extern EventGroupHandle_t g_aliveEvents;
extern TaskHandle_t       g_wdgTaskHandle;
extern TaskHandle_t       g_ledTaskHandle;
extern TaskHandle_t       g_lcdTaskHandle;

/* 1ms 타이머(TIM6) 인터럽트에서 증가하는 카운터 */
extern volatile uint32_t  g_msCounter;
extern volatile uint32_t  g_ledToggleCount;
extern volatile uint32_t  g_wdgRefreshCount;
extern volatile uint8_t   g_resetByWatchdog;

void App_Timer1msISR(void);   /* TIM6 인터럽트 (1ms 마다) 에서 호출 */

void LedTask(void *argument);
void LcdTask(void *argument);
void WdgTask(void *argument);

void Watchdog_Init(void);

#endif /* APP_H */
