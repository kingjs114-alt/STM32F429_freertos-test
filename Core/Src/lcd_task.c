/*
 * lcd_task.c - 2.4" LCD (240x320) 상태 화면
 *
 * 하드웨어 구성 (STM32F429I-DISC1)
 *   ILI9341 : LCD 컨트롤러, SPI5 로 초기화 명령만 보냄
 *   LTDC    : MCU 내장 LCD 컨트롤러, RGB 병렬 신호로 화면 출력
 *   SDRAM   : 프레임버퍼 (0xD0000000), 240x320x4byte = 300KB
 * 이 모든 초기화는 ST 의 BSP (Drivers/BSP/STM32F429I-Discovery) 가 해준다.
 *
 * LCD 는 이 Task 만 사용한다. (여러 Task 가 그리려면 Mutex 로 보호해야 함)
 */
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "app.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

#define LCD_UPDATE_PERIOD_MS    200U
#define LINE_H                  16U     /* Font16 높이 */

static void DrawStaticScreen(void);
static void DrawStatus(void);
static void PrintLine(uint16_t y, uint32_t color, const char *fmt, ...);
static void SimulateHang(void);

void LcdTask(void *argument)
{
    (void)argument;

    BSP_LCD_Init();                                     /* LTDC + ILI9341 + SDRAM */
    BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER, LCD_FRAME_BUFFER);
    BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
    BSP_LCD_DisplayOn();

    DrawStaticScreen();

    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        DrawStatus();

        /* USER 버튼(파란색): 이 Task 를 일부러 멈춰서 워치독 리셋을 확인 */
        if (BSP_PB_GetState(BUTTON_KEY) == GPIO_PIN_SET) {
            SimulateHang();
        }

        xEventGroupSetBits(g_aliveEvents, ALIVE_BIT_LCD);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LCD_UPDATE_PERIOD_MS));
    }
}

static void DrawStaticScreen(void)
{
    BSP_LCD_Clear(LCD_COLOR_BLACK);

    /* 제목 */
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 4, (uint8_t *)"STM32F429 DISCO", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 21, (uint8_t *)"FreeRTOS demo", CENTER_MODE);

    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

    /* 리셋 원인 */
    if (g_resetByWatchdog) {
        PrintLine(48, LCD_COLOR_RED, "Reset: WATCHDOG!");
    } else {
        PrintLine(48, LCD_COLOR_GREEN, "Reset: normal");
    }

    /* 하단 안내 */
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_DisplayStringAt(0, 290, (uint8_t *)"Hold USER button:", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 304, (uint8_t *)"hang LCD task -> WDG reset", CENTER_MODE);
    BSP_LCD_SetFont(&Font16);
}

static void DrawStatus(void)
{
    uint32_t ms  = g_msCounter;
    uint32_t sec = ms / 1000U;
    uint16_t y   = 72;

    PrintLine(y, LCD_COLOR_WHITE,  "Uptime %02lu:%02lu:%02lu",
              sec / 3600U, (sec / 60U) % 60U, sec % 60U);           y += LINE_H;
    PrintLine(y, LCD_COLOR_WHITE,  "1ms IRQ %lu", ms);               y += LINE_H;
    PrintLine(y, LCD_COLOR_WHITE,  "OS tick %lu", (uint32_t)xTaskGetTickCount()); y += LINE_H;
    PrintLine(y, LCD_COLOR_YELLOW, "LED tgl %lu", g_ledToggleCount); y += LINE_H;
    PrintLine(y, LCD_COLOR_CYAN,   "WDG ref %lu", g_wdgRefreshCount); y += LINE_H + 8;

    /* Task 별 남은 스택(최소값, word 단위) - 0 에 가까우면 스택을 늘려야 함 */
    PrintLine(y, LCD_COLOR_LIGHTGRAY, "Free stack(word)");           y += LINE_H;
    PrintLine(y, LCD_COLOR_LIGHTGRAY, " WDG:%-4lu LED:%-4lu",
              (uint32_t)uxTaskGetStackHighWaterMark(g_wdgTaskHandle),
              (uint32_t)uxTaskGetStackHighWaterMark(g_ledTaskHandle)); y += LINE_H;
    PrintLine(y, LCD_COLOR_LIGHTGRAY, " LCD:%-4lu",
              (uint32_t)uxTaskGetStackHighWaterMark(NULL));          y += LINE_H;
    PrintLine(y, LCD_COLOR_LIGHTGRAY, "Heap free %lu",
              (uint32_t)xPortGetFreeHeapSize());                     y += LINE_H + 8;

    /* LED3 상태 표시 (실제 LED 와 같이 깜박임) */
    uint8_t ledOn = (HAL_GPIO_ReadPin(LED3_GPIO_PORT, LED3_PIN) == GPIO_PIN_SET);
    BSP_LCD_SetTextColor(ledOn ? LCD_COLOR_GREEN : LCD_COLOR_DARKGRAY);
    BSP_LCD_FillCircle(BSP_LCD_GetXSize() / 2, y + 14, 12);
}

/* 한 줄 출력. 이전 글자가 남지 않게 줄 끝까지 공백으로 채운다. */
static void PrintLine(uint16_t y, uint32_t color, const char *fmt, ...)
{
    char text[32];
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    if (n < 0) {
        n = 0;
    }
    for (; n < (int)sizeof(text) - 1; n++) {
        text[n] = ' ';
    }
    text[sizeof(text) - 1] = '\0';

    BSP_LCD_SetTextColor(color);
    BSP_LCD_DisplayStringAt(4, y, (uint8_t *)text, LEFT_MODE);
}

static void SimulateHang(void)
{
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(0, 250, BSP_LCD_GetXSize(), 34);
    BSP_LCD_SetBackColor(LCD_COLOR_RED);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 259, (uint8_t *)"LCD task HANG!", CENTER_MODE);

    /*
     * 무한 루프 = 이 Task 가 alive 비트를 더 이상 보고하지 못함.
     * LED 는 계속 깜박이지만(더 높은 우선순위) WdgTask 가 이를 감지하고
     * IWDG refresh 를 멈춘다 -> 약 4초 후 리셋 -> 화면에 "Reset: WATCHDOG!"
     */
    for (;;) {
    }
}
