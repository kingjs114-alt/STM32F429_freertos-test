/*
 * main.c - STM32F429I-DISC1 + FreeRTOS 예제
 *
 *  - TIM6  : 1ms 타이머 인터럽트 (HAL tick + 앱 1ms 처리)
 *  - LED3  : (녹색, PG13) 1초 간격 깜박임
 *  - LED4  : (빨강, PG14) 이전 리셋이 워치독에 의한 것이면 켜짐 / 에러 시 켜짐
 *  - LCD   : 2.4" 240x320 (ILI9341 + LTDC + SDRAM) 상태 표시
 *  - IWDG  : 독립 워치독, 모든 Task 가 살아있을 때만 refresh
 *
 * Task 구성 (자세한 설명은 README.md)
 *   WdgTask (prio 3) : 워치독 감시
 *   LedTask (prio 2) : 1초 LED 토글 (TIM6 ISR 이 1000ms 마다 깨움)
 *   LcdTask (prio 1) : LCD 화면 갱신
 */
#include "main.h"
#include "app.h"
#include "stm32f429i_discovery.h"

EventGroupHandle_t g_aliveEvents;
TaskHandle_t       g_wdgTaskHandle;
TaskHandle_t       g_ledTaskHandle;
TaskHandle_t       g_lcdTaskHandle;
volatile uint8_t   g_resetByWatchdog;

static void SystemClock_Config(void);
static void CheckResetCause(void);

int main(void)
{
    /* HAL 초기화: Flash 캐시, NVIC 그룹, 1ms 타이머(TIM6) 시작 (HAL_InitTick) */
    HAL_Init();

    /* 시스템 클럭 180MHz (HSE 8MHz -> PLL) */
    SystemClock_Config();

    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

    CheckResetCause();

    /* Task 들 사이에서 공유하는 객체는 Task 를 만들기 전에 생성 */
    g_aliveEvents = xEventGroupCreate();
    configASSERT(g_aliveEvents != NULL);

    /* Task 생성 */
    BaseType_t ok = pdPASS;
    ok &= xTaskCreate(WdgTask, "WDG", WDG_TASK_STACK, NULL, WDG_TASK_PRIO, &g_wdgTaskHandle);
    ok &= xTaskCreate(LedTask, "LED", LED_TASK_STACK, NULL, LED_TASK_PRIO, &g_ledTaskHandle);
    ok &= xTaskCreate(LcdTask, "LCD", LCD_TASK_STACK, NULL, LCD_TASK_PRIO, &g_lcdTaskHandle);
    configASSERT(ok == pdPASS);

    /*
     * 주의: 첫 FreeRTOS API 호출 이후부터 스케줄러 시작 전까지는
     * 우선순위 5~15 인터럽트가 막혀 있으므로 여기서 HAL_Delay() 를 쓰면 멈춘다.
     * 주변장치 초기화(LCD 등)는 각 Task 안에서 하자.
     */
    vTaskStartScheduler();

    /* 여기에 도달했다면 heap 부족으로 Idle Task 를 못 만든 것 */
    Error_Handler();
}

/*
 * 이전 리셋 원인 확인 (워치독 리셋이었는지)
 */
static void CheckResetCause(void)
{
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
        g_resetByWatchdog = 1;
        BSP_LED_On(LED4);
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

/*
 * System Clock
 *   SYSCLK = 180 MHz, HCLK = 180 MHz, APB1 = 45 MHz, APB2 = 90 MHz
 *   HSE 8 MHz / M(8) * N(360) / P(2) = 180 MHz
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 8;
    osc.PLL.PLLN       = 360;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    /* 180MHz 동작을 위해 Over-Drive 활성화 */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/*------------------------------------------------------------------------
 * 에러 처리 / FreeRTOS Hook
 * 빨간 LED 를 켜고 멈춘다. 워치독이 refresh 되지 않으므로 몇 초 후 자동 리셋.
 *----------------------------------------------------------------------*/
void Error_Handler(void)
{
    __disable_irq();
    HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, GPIO_PIN_SET);
    for (;;) {
    }
}

void vAssertCalled(const char *file, int line)
{
    (void)file;
    (void)line;
    Error_Handler();   /* 디버거로 file / line 값을 확인하세요 */
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;  /* 어떤 Task 의 스택이 넘쳤는지 디버거로 확인 */
    Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
    /* configTOTAL_HEAP_SIZE 를 늘리거나 Task 스택을 줄이세요 */
    Error_Handler();
}
