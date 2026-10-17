/*
 * stm32f4xx_it.c - 인터럽트 핸들러
 *
 * SVC_Handler / PendSV_Handler / SysTick_Handler 는 FreeRTOS 포트(port.c)가
 * 제공하므로 여기에 정의하지 않는다. (FreeRTOSConfig.h 참고)
 */
#include "main.h"
#include "stm32f4xx_it.h"

extern TIM_HandleTypeDef htim6;

void NMI_Handler(void)
{
    for (;;) {
    }
}

void HardFault_Handler(void)
{
    Error_Handler();
}

void MemManage_Handler(void)
{
    Error_Handler();
}

void BusFault_Handler(void)
{
    Error_Handler();
}

void UsageFault_Handler(void)
{
    Error_Handler();
}

void DebugMon_Handler(void)
{
}

/* 1ms 타이머 인터럽트 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}
