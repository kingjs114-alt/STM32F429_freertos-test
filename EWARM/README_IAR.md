# IAR Embedded Workbench for Arm 9.3 에서 빌드/굽기

이 폴더에는 IAR 전용 파일이 들어 있습니다. 소스 코드(`Core/`, `Drivers/`, `Middlewares/`)는 GCC 빌드와 같은 것을 씁니다.

| 파일 | 설명 |
|---|---|
| `startup_stm32f429xx.s` | IAR용 startup (벡터 테이블). GCC용 `Core/Startup/...s`는 쓰지 않음 |
| `stm32f429xx_flash.icf` | IAR 링커 설정 (CSTACK 0x800, HEAP 0x200) |
| FreeRTOS IAR 포트 | `Middlewares/Third_Party/FreeRTOS/Source/portable/IAR/ARM_CM4F/` |

아래 순서대로 **한 번만** 프로젝트를 만들면, 이후에는 `F7`(빌드) → `Ctrl+D`(굽기+디버그)로 끝입니다.

---

## 1. 프로젝트 만들기

1. **Project → Create New Project…** → Tool chain `Arm` → **Empty project** → OK
2. 저장 위치를 **이 `EWARM` 폴더**로 하고 이름은 `stm32f429_freertos.ewp`
   (아래 경로들은 모두 `$PROJ_DIR$` = 이 `EWARM` 폴더 기준입니다)
3. **File → Save Workspace** → 같은 폴더에 `stm32f429_freertos.eww`

## 2. 파일 추가

Workspace 창에서 프로젝트 이름 우클릭 → **Add → Add Group…** 으로 그룹을 만들고,
그룹 우클릭 → **Add → Add Files…** 로 아래 파일들을 넣습니다.

| 그룹 | 추가할 파일 |
|---|---|
| `Startup` | `EWARM\startup_stm32f429xx.s` |
| `App` | `Core\Src\` 안의 **.c 전부** (syscalls.c / sysmem.c 는 GCC 전용이라 IAR 에서는 자동으로 빈 파일이 됨) |
| `HAL` | `Drivers\STM32F4xx_HAL_Driver\Src\` 에서 아래 파일 |
| `BSP` | `Drivers\BSP\STM32F429I-Discovery\stm32f429i_discovery.c`<br>`Drivers\BSP\STM32F429I-Discovery\stm32f429i_discovery_lcd.c`<br>`Drivers\BSP\STM32F429I-Discovery\stm32f429i_discovery_sdram.c`<br>`Drivers\BSP\Components\ili9341\ili9341.c` |
| `FreeRTOS` | `Middlewares\Third_Party\FreeRTOS\Source\` 의<br>`tasks.c` `queue.c` `list.c` `timers.c` `event_groups.c` `stream_buffer.c`<br>`portable\IAR\ARM_CM4F\port.c`<br>`portable\IAR\ARM_CM4F\portasm.s`<br>`portable\MemMang\heap_4.c` |

`HAL` 그룹에 넣을 파일 (`stm32f4xx_` 접두어 생략):
```
hal.c          hal_cortex.c   hal_rcc.c      hal_rcc_ex.c   hal_gpio.c
hal_exti.c     hal_dma.c      hal_dma_ex.c   hal_pwr.c      hal_pwr_ex.c
hal_flash.c    hal_flash_ex.c hal_flash_ramfunc.c
hal_tim.c      hal_tim_ex.c   hal_iwdg.c     hal_spi.c      hal_i2c.c
hal_i2c_ex.c   hal_ltdc.c     hal_ltdc_ex.c  hal_dma2d.c    hal_sdram.c
ll_fmc.c
```

> **넣으면 안 되는 것**
> - `Core\Startup\startup_stm32f429xx.s` (GCC용)
> - `portable\GCC\...` (GCC용 FreeRTOS 포트)
> - `Utilities\Fonts\*.c` (lcd.c 안에서 `#include` 하므로 따로 넣으면 중복 정의 에러)
> - HAL의 `*_template.c` 파일

## 3. 옵션 설정

프로젝트 이름 우클릭 → **Options…** (`Alt+F7`)

### General Options
- **Target** 탭 → Device: `ST` → `STM32F4` → **`STM32F429ZI`**
  - FPU 가 `VFPv4 single precision` 으로 잡혀 있는지 확인 (FreeRTOS ARM_CM4F 포트에 필요)
- **Library Configuration** 탭 → Library: `Normal`
- **Library Options 1** 탭 → Printf formatter: `Full` 또는 `Auto` (LCD 출력에 `vsnprintf` 사용)

### C/C++ Compiler
- **Language 1** 탭 → Language: `C`, C dialect: `Standard C` (C11 이상)
- **Preprocessor** 탭 → *Additional include directories* (한 줄에 하나씩 그대로 붙여넣기):
  ```
  $PROJ_DIR$\..\Core\Inc
  $PROJ_DIR$\..\Drivers\STM32F4xx_HAL_Driver\Inc
  $PROJ_DIR$\..\Drivers\CMSIS\Device\ST\STM32F4xx\Include
  $PROJ_DIR$\..\Drivers\CMSIS\Include
  $PROJ_DIR$\..\Drivers\BSP\STM32F429I-Discovery
  $PROJ_DIR$\..\Drivers\BSP\Components\ili9341
  $PROJ_DIR$\..\Middlewares\Third_Party\FreeRTOS\Source\include
  $PROJ_DIR$\..\Middlewares\Third_Party\FreeRTOS\Source\portable\IAR\ARM_CM4F
  ```
- 같은 탭 → *Defined symbols*:
  ```
  STM32F429xx
  USE_HAL_DRIVER
  ```

### Assembler
- **Preprocessor** 탭 → *Additional include directories*:
  ```
  $PROJ_DIR$\..\Core\Inc
  ```
  (FreeRTOS `portasm.s` 가 `FreeRTOSConfig.h` 를 include 합니다)

### Output Converter (선택)
- **Generate additional output** 체크 → `Intel hex` 또는 `Raw binary`

### Linker
- **Config** 탭 → *Override default* 체크 → `$PROJ_DIR$\stm32f429xx_flash.icf`

### Debugger
- **Setup** 탭 → Driver: **`ST-LINK`**
- **Download** 탭 → **Use flash loader(s)** 체크
- **ST-LINK** 항목 → **Setup** 탭 → Interface: **SWD**, Reset: `Connect during reset` (안 되면 `System`)

## 4. 빌드 / 굽기 / 실행

1. 보드의 **ST-LINK 쪽 USB** (Mini-USB 또는 Micro-USB, 보드 버전에 따라 다름)를 PC 에 연결
2. **Project → Make** (`F7`): 빌드
3. **Project → Download and Debug** (`Ctrl+D`): 보드에 굽고 디버거 시작 (`main()` 에서 멈춤)
4. **Debug → Go** (`F5`): 실행 → 녹색 LED 1초 깜박임, LCD 상태 화면 표시

디버거 없이 굽기만 하려면 **Project → Download → Download active application**.

### 자주 만나는 문제
| 증상 | 해결 |
|---|---|
| `Error[Li005]: no definition for "xxx"` | 2단계의 파일이 빠졌는지 확인 |
| `Error[Li006]: duplicate definitions for "Font24"` 등 | `Utilities\Fonts\*.c` 를 프로젝트에서 빼기 |
| `Fatal Error[Pe1696]: cannot open source file "FreeRTOSConfig.h"` (portasm.s) | Assembler 의 include 경로 추가 |
| `#error This port can only be used when the project options are configured to enable hardware floating point support` | General Options → Target 에서 FPU 를 VFPv4 single precision 으로 |
| ST-LINK 연결 실패 | ST-LINK 드라이버 설치, Reset 을 `Connect during reset` 으로 |
| 부팅 후 몇 초마다 계속 리셋됨 (빨간 LED 켜짐) | 워치독 리셋입니다. LCD 초기화 실패 등으로 Task 가 멈췄을 수 있으니 디버거로 `Error_Handler` / `vAssertCalled` 에 브레이크포인트를 걸어 확인 |
