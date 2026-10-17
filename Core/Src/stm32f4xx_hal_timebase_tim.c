/*
 * stm32f4xx_hal_timebase_tim.c - TIM6 를 이용한 1ms 타이머 인터럽트
 *
 * FreeRTOS 는 SysTick 을 OS tick 으로 사용하므로, HAL 의 tick(HAL_Delay,
 * 타임아웃 등)은 TIM6 으로 옮긴다. (CubeMX 가 FreeRTOS 사용 시 하는 것과 동일)
 * 이 1ms 인터럽트에서 앱 처리(App_Timer1msISR)도 함께 수행한다.
 */
#include "main.h"
#include "app.h"

TIM_HandleTypeDef htim6;

/* HAL_Init() / HAL_RCC_ClockConfig() 에서 자동 호출됨 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t           flashLatency;
    uint32_t           timClock;

    __HAL_RCC_TIM6_CLK_ENABLE();

    /* APB1 분주비가 1 이 아니면 타이머 클럭은 PCLK1 x 2 */
    HAL_RCC_GetClockConfig(&clkconfig, &flashLatency);
    if (clkconfig.APB1CLKDivider == RCC_HCLK_DIV1) {
        timClock = HAL_RCC_GetPCLK1Freq();
    } else {
        timClock = 2U * HAL_RCC_GetPCLK1Freq();
    }

    /* 타이머 카운터 1MHz, 1000 카운트 = 1ms */
    htim6.Instance               = TIM6;
    htim6.Init.Prescaler         = (timClock / 1000000U) - 1U;
    htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim6.Init.Period            = 1000U - 1U;
    htim6.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        return HAL_ERROR;
    }

    if (TickPriority >= (1UL << __NVIC_PRIO_BITS)) {
        return HAL_ERROR;
    }
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TickPriority, 0U);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    uwTickPrio = TickPriority;

    return HAL_TIM_Base_Start_IT(&htim6);
}

void HAL_SuspendTick(void)
{
    __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
    __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}

/* 1ms 마다 호출되는 타이머 콜백 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        HAL_IncTick();
        App_Timer1msISR();
    }
}
