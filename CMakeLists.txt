cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

project(stm32f429_freertos C ASM)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

#-----------------------------------------------------------------------------
# 소스
#-----------------------------------------------------------------------------
set(HAL_DIR      Drivers/STM32F4xx_HAL_Driver)
set(BSP_DIR      Drivers/BSP/STM32F429I-Discovery)
set(FREERTOS_DIR Middlewares/Third_Party/FreeRTOS/Source)

file(GLOB APP_SOURCES Core/Src/*.c)

set(HAL_MODULES
    hal hal_cortex hal_rcc hal_rcc_ex hal_gpio hal_exti hal_dma hal_dma_ex
    hal_pwr hal_pwr_ex hal_flash hal_flash_ex hal_flash_ramfunc
    hal_tim hal_tim_ex hal_iwdg hal_spi hal_i2c hal_i2c_ex
    hal_ltdc hal_ltdc_ex hal_dma2d hal_sdram ll_fmc
)
list(TRANSFORM HAL_MODULES PREPEND ${HAL_DIR}/Src/stm32f4xx_)
list(TRANSFORM HAL_MODULES APPEND .c)

set(BSP_SOURCES
    ${BSP_DIR}/stm32f429i_discovery.c
    ${BSP_DIR}/stm32f429i_discovery_lcd.c
    ${BSP_DIR}/stm32f429i_discovery_sdram.c
    Drivers/BSP/Components/ili9341/ili9341.c
)

set(FREERTOS_SOURCES
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

add_executable(${PROJECT_NAME}
    Core/Startup/startup_stm32f429xx.s
    ${APP_SOURCES}
    ${HAL_MODULES}
    ${BSP_SOURCES}
    ${FREERTOS_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    ${HAL_DIR}/Inc
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    ${BSP_DIR}
    Drivers/BSP/Components/ili9341
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32F429xx
    USE_HAL_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -Wall
    -fdata-sections -ffunction-sections
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-O2 -g>
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F429ZITX_FLASH.ld
    --specs=nano.specs
    --specs=nosys.specs
    -Wl,-Map=${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,--no-warn-rwx-segments
)

target_link_libraries(${PROJECT_NAME} PRIVATE c m)

# .elf -> .hex / .bin 생성
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)
