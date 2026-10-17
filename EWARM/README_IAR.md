# IAR Embedded Workbench for Arm 9.3 에서 빌드/굽기

이 폴더에 IAR 프로젝트가 준비되어 있습니다. **`stm32f429_freertos.eww` 를 더블클릭해서 열면 됩니다.**

| 파일 | 설명 |
|---|---|
| `stm32f429_freertos.eww` | Workspace (이걸 열기) |
| `stm32f429_freertos.ewp` | 프로젝트: 소스 파일 목록, 컴파일/링크 옵션 |
| `stm32f429_freertos.ewd` | 디버거 설정: ST-LINK, SWD |
| `startup_stm32f429xx.s` | IAR용 startup (벡터 테이블) |
| `stm32f429xx_flash.icf` | IAR 링커 설정 (CSTACK 0x800, HEAP 0x200) |

> 프로젝트 파일은 ST 의 STM32CubeF4 공식 IAR 예제(STM32F429I-Discovery FreeRTOS)를 바탕으로 만들었습니다.
> 처음 열 때 IAR 이 **"프로젝트를 새 버전 형식으로 변환할까요?"** 라고 물으면 **OK/Yes** 를 누르세요.
> 변환 후 저장하면 다음부터는 묻지 않습니다.

---

## 1. 빌드 / 굽기 / 실행

1. 보드의 **ST-LINK 쪽 USB** 를 PC 에 연결 (ST-LINK 드라이버는 IAR 설치 시 함께 설치됨)
2. `stm32f429_freertos.eww` 열기
3. **Project → Make** (`F7`): 빌드
4. **Project → Download and Debug** (`Ctrl+D`): 보드에 굽고 디버거 시작 (`main()` 에서 멈춤)
5. **Debug → Go** (`F5`): 실행
   - 녹색 LED3 이 1초 간격으로 깜박이고 LCD 에 상태 화면이 나오면 성공

디버거 없이 굽기만 하려면 **Project → Download → Download active application**.
디버그를 끝내려면 **Debug → Stop Debugging** (`Ctrl+Shift+D`). 보드는 계속 동작합니다.

### 결과물 위치
- `EWARM\Debug\Exe\stm32f429_freertos.out` (디버그용 ELF)
- `EWARM\Debug\Exe\stm32f429_freertos.hex` (다른 프로그래머로 굽기용)

---

## 2. 미리 설정되어 있는 것

**General Options**
- Device: `ST STM32F429ZI`, FPU: `VFPv4 single precision` (FreeRTOS ARM_CM4F 포트에 필요)
- Library: `Full` DLIB

**C/C++ Compiler**
- 정의: `USE_HAL_DRIVER`, `STM32F429xx`
- 최적화: `Low` (디버깅하기 쉬움. 출시용은 Options → C/C++ Compiler → Optimizations 에서 High 로)
- include 경로:
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

**Assembler**: include 경로 `$PROJ_DIR$\..\Core\Inc` (FreeRTOS `portasm.s` 가 `FreeRTOSConfig.h` 를 읽음)

**Linker**: `$PROJ_DIR$\stm32f429xx_flash.icf`

**Output Converter**: Intel hex 생성

**Debugger**: ST-LINK, SWD, flash loader 사용, `main` 까지 실행 후 정지

### 프로젝트 그룹
| 그룹 | 내용 |
|---|---|
| `Startup` | `startup_stm32f429xx.s` |
| `App` | `Core\Src\*.c` (GCC 전용인 `syscalls.c`, `sysmem.c` 제외) |
| `HAL` | 사용하는 HAL 드라이버 24개 |
| `BSP` | 보드 드라이버 (LCD, SDRAM, ILI9341) |
| `FreeRTOS` | 커널 + IAR ARM_CM4F 포트 + heap_4 |
| `Config` | `FreeRTOSConfig.h`, `stm32f4xx_hal_conf.h`, `app.h` (편집하기 쉽게 표시만) |

새 `.c` 파일을 만들면 `App` 그룹 우클릭 → **Add → Add Files…** 로 추가하세요.
(GCC 빌드는 `Core/Src/*.c` 를 자동으로 포함합니다)

---

## 3. 자주 만나는 문제

| 증상 | 해결 |
|---|---|
| 열 때 "converting project" 메시지 | 정상입니다. OK 누르고 저장 |
| `Error[Li005]: no definition for "xxx"` | 새로 만든 `.c` 파일을 프로젝트에 추가했는지 확인 |
| `Error[Li006]: duplicate definitions for "Font24"` 등 | `Utilities\Fonts\*.c` 를 프로젝트에 넣지 마세요 (lcd.c 가 `#include` 함) |
| ST-LINK 연결 실패 | USB 케이블/포트 확인, Options → Debugger → ST-LINK → Reset 을 `Connect during reset` 으로 |
| ST-LINK 펌웨어 업데이트 요청 | 안내에 따라 업데이트 (또는 STM32CubeProgrammer 의 ST-LINK upgrade) |
| 부팅 후 몇 초마다 계속 리셋 (빨간 LED4 켜짐) | 워치독 리셋입니다. Task 가 멈췄을 수 있으니 `Error_Handler` / `vAssertCalled` 에 브레이크포인트를 걸어 확인 |
