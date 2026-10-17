/*
 * stm32f4xx_hal_msp.c - HAL 전역 MSP 초기화
 */
#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* FreeRTOS 는 서브 우선순위를 쓰지 않으므로 4bit 모두 선점 우선순위로 */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* PendSV 는 가장 낮은 우선순위 (FreeRTOS 가 다시 설정하지만 명시) */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
}
