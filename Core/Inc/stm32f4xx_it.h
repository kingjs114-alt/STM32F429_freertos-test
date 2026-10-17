/*
 * stm32f4xx_it.h - 인터럽트 핸들러 선언
 */
#ifndef STM32F4XX_IT_H
#define STM32F4XX_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void TIM6_DAC_IRQHandler(void);

#endif /* STM32F4XX_IT_H */
