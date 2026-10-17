# STM32F429I-DISC1 FreeRTOS 예제

STM32F429 Discovery 보드에서 FreeRTOS를 처음 써 보는 사람을 위한 예제입니다.

| 기능 | 구현 |
|---|---|
| 1ms 타이머 인터럽트 | **TIM6** (1MHz 카운트, 1000 카운트 = 1ms) |
| 1초 LED 깜박임 | **LED3 (녹색, PG13)**: 1ms 인터럽트가 1000번 세면 LedTask를 깨움 |
| 워치독 | **IWDG** (약 4초): 모든 Task가 살아있을 때만 refresh |
| LCD | **2.4" 240x320** (ILI9341 + LTDC + SDRAM 프레임버퍼): 상태 화면 |
| 에러 표시 | **LED4 (빨강, PG14)**: 워치독 리셋 후 부팅했거나 에러가 나면 켜짐 |

---

## 1. Task는 몇 개가 좋을까?

> **원칙: "서로 다른 주기/타이밍으로 독립적으로 돌아야 하는 일" 하나당 Task 하나.**
> 너무 잘게 나누면 스택 메모리 낭비, Task 간 동기화 문제만 늘어납니다.

이 예제는 **애플리케이션 Task 3개**로 구성했습니다.

| Task | 우선순위 | 주기 | 하는 일 | 왜 따로 만들었나 |
|---|---|---|---|---|
| `WdgTask` | **3 (최고)** | 1.5초 | 다른 Task가 살아있는지 확인 후 IWDG refresh | 다른 Task가 바빠도 감시는 반드시 돌아야 함 |
| `LedTask` | 2 | 1초 | LED3 토글 | 짧고 시간이 정확해야 함 |
| `LcdTask` | 1 (최저) | 200ms | LCD 상태 화면 갱신 | 그리기는 오래 걸림. 낮은 우선순위라 다른 Task를 방해하지 않음 |

FreeRTOS가 자동으로 만드는 Task도 2개 있습니다.

- **Idle Task** (우선순위 0): 할 일이 없을 때 실행
- **Timer Service Task** (우선순위 6): 소프트웨어 타이머용 (`configUSE_TIMERS`)

**우선순위 정하는 요령**: 처리 시간이 짧고 시간을 지켜야 하는 일은 높게, 오래 걸리고 덜 급한 일(화면, 로그)은 낮게.
감시자(워치독)는 가장 높게 둡니다.

나중에 기능을 추가할 때의 예:
- UART 명령 처리 → `UartTask` 추가 (수신 인터럽트 → Queue → Task)
- 터치스크린 → `TouchTask` 추가, 그리기 요청은 Queue로 `LcdTask`에 전달
- 센서 주기 측정 → `SensorTask` 추가

새 Task를 추가하면 `app.h`에 `ALIVE_BIT_xxx`를 하나 더 만들고 `ALIVE_BITS_ALL`에 포함시키세요. 그러면 워치독 감시 대상이 됩니다.

---

## 2. 동작 흐름

```
 TIM6 인터럽트 (1ms)                       FreeRTOS Task
 ───────────────────                      ──────────────────────────────────
 HAL_IncTick()           (HAL_Delay용)
 g_msCounter++
 1000번째?  ──vTaskNotifyGiveFromISR──▶  LedTask  : LED3 토글
                                                    └─ alive 비트(LED) set ─┐
                                          LcdTask  : 200ms마다 화면 갱신      │
                                                    └─ alive 비트(LCD) set ─┤
                                                                            ▼
                                          WdgTask  : 1.5초마다 Event Group 확인
                                                    모든 비트 set? → IWDG refresh
                                                    하나라도 없음? → refresh 안 함 → 약 4초 뒤 리셋
```

### 워치독 테스트
1. 보드의 **파란색 USER 버튼**을 누르고 있으면 `LcdTask`가 일부러 무한 루프에 빠집니다.
2. LED는 계속 깜박이지만(다른 Task라서), `WdgTask`가 LCD Task의 응답이 없다는 것을 감지합니다.
3. 몇 초 뒤 MCU가 리셋되고, 화면에 **`Reset: WATCHDOG!`**이 나오며 빨간 LED4가 켜집니다.

> 디버거로 정지(breakpoint)해 있는 동안에는 워치독이 멈추도록 설정했습니다 (`__HAL_DBGMCU_FREEZE_IWDG()`).

---

## 3. 폴더 구조

```
Core/
  Inc/FreeRTOSConfig.h          FreeRTOS 설정 (tick 1ms, heap 32KB, 인터럽트 우선순위)
  Inc/stm32f4xx_hal_conf.h      HAL 설정 (사용 모듈, HSE 8MHz)
  Inc/app.h                     Task 우선순위/스택 크기/alive 비트 정의
  Src/main.c                    클럭 설정(180MHz), Task 생성, 에러 Hook
  Src/stm32f4xx_hal_timebase_tim.c   TIM6 1ms 타이머 인터럽트
  Src/app_timer.c               1ms 인터럽트에서 하는 일
  Src/led_task.c                LedTask
  Src/lcd_task.c                LcdTask
  Src/wdg_task.c                WdgTask + IWDG 설정
  Src/stm32f4xx_it.c            인터럽트 핸들러
  Startup/startup_stm32f429xx.s 벡터 테이블, 리셋 핸들러
Drivers/                        ST CMSIS, HAL, BSP(LCD/SDRAM), ILI9341 드라이버
Middlewares/Third_Party/FreeRTOS  FreeRTOS-Kernel V11.1.0 (ARM_CM4F 포트, heap_4)
Utilities/Fonts                 LCD 폰트
STM32F429ZITX_FLASH.ld          링커 스크립트
CMakeLists.txt
```

외부 소스 버전
- STM32CubeF4 HAL / CMSIS / 32F429IDISCOVERY BSP / ILI9341: STM32CubeF4 저장소의 서브모듈 커밋 그대로
- FreeRTOS-Kernel: V11.1.0

---

## 4. 빌드

### 명령줄 (CMake + Arm GNU Toolchain)
필요: `arm-none-eabi-gcc`, `cmake` (3.22 이상), `make` 또는 `ninja`

```bash
cmake -B build                     # 기본 Debug. Release: -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
결과물: `build/stm32f429_freertos.elf`, `.hex`, `.bin`

### VS Code
STM32 VS Code Extension 또는 CMake Tools 확장으로 이 폴더를 열면 `CMakeLists.txt`를 그대로 사용합니다.

## 5. 굽기 (Flash)

보드를 USB(ST-LINK 쪽 Mini-USB)로 연결한 뒤 하나를 선택:

```bash
# STM32CubeProgrammer CLI
STM32_Programmer_CLI -c port=SWD -w build/stm32f429_freertos.elf -v -rst

# OpenOCD
openocd -f board/stm32f429disc1.cfg -c "program build/stm32f429_freertos.elf verify reset exit"

# st-flash (stlink 도구)
st-flash --reset write build/stm32f429_freertos.bin 0x08000000
```

> 보드가 USB 드라이브로 보인다면 `.bin` 파일을 그 드라이브에 복사해도 됩니다.

---

## 6. FreeRTOS 처음 쓸 때 꼭 알아야 할 것

1. **Task 안에서 `HAL_Delay()` 대신 `vTaskDelay()`/`vTaskDelayUntil()`**
   `HAL_Delay`는 CPU를 붙잡고 기다리지만, `vTaskDelay`는 그동안 다른 Task를 실행시킵니다.
   정확한 주기가 필요하면 `vTaskDelayUntil`을 쓰세요 (이 예제의 WdgTask, LcdTask).
2. **모든 Task는 `for (;;)` 무한 루프 안에서 반드시 한 번은 Block 되어야 함**
   (`vTaskDelay`, `ulTaskNotifyTake`, Queue 대기 등). 안 그러면 낮은 우선순위 Task가 영원히 못 돕니다.
3. **인터럽트 안에서는 `...FromISR()` API만** 사용하고, 그 인터럽트의 우선순위는 **5~15**여야 합니다.
   (`FreeRTOSConfig.h`의 `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`)
4. **SysTick은 FreeRTOS 차지** → HAL tick은 TIM6으로 옮겼습니다.
5. **`vTaskStartScheduler()` 전에는 `HAL_Delay()` 금지** - 첫 `xTaskCreate()` 이후 인터럽트가 막혀 있어 멈춥니다.
   그래서 LCD 초기화는 `LcdTask` 안에서 합니다.
6. **스택 크기 확인**: LCD 화면의 `Free stack` 값(남은 최소 스택, word 단위)이 0에 가까워지면 `app.h`의 스택 크기를 늘리세요.
   `configCHECK_FOR_STACK_OVERFLOW = 2`로 넘치면 `vApplicationStackOverflowHook()`에서 멈춥니다.
7. **공유 자원**: LCD는 `LcdTask` 하나만 사용합니다. 여러 Task가 같은 주변장치를 쓰려면 Mutex로 보호하세요.

### Task 간 통신 방법 정리
| 방법 | 언제 쓰나 | 이 예제 |
|---|---|---|
| Task Notification | ISR/Task → 특정 Task 하나 깨우기 (가장 가볍고 빠름) | TIM6 ISR → LedTask |
| Event Group | 여러 조건(비트)을 한 번에 확인 | alive 비트 → WdgTask |
| Queue | 데이터를 주고받기 | (확장 시) UART 수신 데이터 |
| Mutex | 공유 자원 보호 | (확장 시) LCD를 여러 Task가 쓸 때 |
