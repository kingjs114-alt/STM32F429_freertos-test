/*
 * FreeRTOSConfig.h - STM32F429I-DISC1 (Cortex-M4F, 180 MHz)
 *
 * FreeRTOS 커널 동작을 결정하는 설정 파일입니다.
 * 자주 만지게 되는 항목에는 한글 주석을 달아 두었습니다.
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

/*-------------------------------------------------------------------------
 * 스케줄러 기본 설정
 *-----------------------------------------------------------------------*/
#define configUSE_PREEMPTION                    1   /* 선점형: 높은 우선순위 Task가 준비되면 즉시 전환 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      ((TickType_t)1000) /* OS tick = 1ms (SysTick) */
#define configMAX_PRIORITIES                    7   /* 우선순위 0(Idle) ~ 6 */
#define configMINIMAL_STACK_SIZE                ((uint16_t)128) /* 단위: word(4byte) */
#define configMAX_TASK_NAME_LEN                 16
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TIME_SLICING                  1

/*-------------------------------------------------------------------------
 * 메모리 (heap_4.c 사용)
 *-----------------------------------------------------------------------*/
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configTOTAL_HEAP_SIZE                   ((size_t)(32 * 1024)) /* xTaskCreate 등이 여기서 할당 */

/*-------------------------------------------------------------------------
 * 기능 on/off
 *-----------------------------------------------------------------------*/
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_EVENT_GROUPS                  1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_CO_ROUTINES                   0
#define configUSE_NEWLIB_REENTRANT              0

/* 소프트웨어 타이머 (Timer Service Task) */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/*-------------------------------------------------------------------------
 * Hook / 디버깅 - 처음 배울 때는 켜 두는 것을 강력히 추천
 *-----------------------------------------------------------------------*/
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1   /* heap 부족 시 vApplicationMallocFailedHook() */
#define configCHECK_FOR_STACK_OVERFLOW          2   /* stack overflow 시 vApplicationStackOverflowHook() */
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configGENERATE_RUN_TIME_STATS           0
#define configRECORD_STACK_HIGH_ADDRESS         1

/*-------------------------------------------------------------------------
 * 포함할 API
 *-----------------------------------------------------------------------*/
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          1

/*-------------------------------------------------------------------------
 * Cortex-M 인터럽트 우선순위
 *
 * STM32F4 는 우선순위 비트가 4개 (0 = 가장 높음, 15 = 가장 낮음).
 * - 우선순위 0~4 인터럽트 : FreeRTOS가 절대 막지 않음. 단, FreeRTOS API 호출 금지!
 * - 우선순위 5~15 인터럽트 : "...FromISR()" API 호출 가능.
 *-----------------------------------------------------------------------*/
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS                         __NVIC_PRIO_BITS
#else
#define configPRIO_BITS                         4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* configASSERT: 잘못 사용하면 여기서 멈춤 -> 디버거로 호출 스택 확인
 * (이 파일은 IAR 어셈블러(portasm.s)도 include 하므로 C 선언은 C 컴파일러일 때만) */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
void vAssertCalled(const char *file, int line);
#endif
#define configASSERT(x) if ((x) == 0) { vAssertCalled(__FILE__, __LINE__); }

/*-------------------------------------------------------------------------
 * FreeRTOS 포트 핸들러를 CMSIS 표준 이름에 연결
 * (stm32f4xx_it.c 에 SVC/PendSV/SysTick 핸들러를 따로 만들면 안 됨)
 *-----------------------------------------------------------------------*/
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
