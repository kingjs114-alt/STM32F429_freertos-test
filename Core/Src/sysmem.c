/*
 * sysmem.c - newlib malloc 용 _sbrk (snprintf 등이 내부적으로 사용할 수 있음)
 * FreeRTOS Task 들은 heap_4 (configTOTAL_HEAP_SIZE) 를 사용한다.
 */
/* GCC(newlib) 전용. IAR 은 자체 라이브러리(DLIB)를 쓰므로 컴파일되지 않음 */
#if defined(__GNUC__) && !defined(__ICCARM__)

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

void *_sbrk(ptrdiff_t incr);

void *_sbrk(ptrdiff_t incr)
{
    extern uint8_t _end;              /* 링커 스크립트: .bss 끝 */
    extern uint8_t _estack;           /* 링커 스크립트: RAM 끝 */
    extern uint32_t _Min_Stack_Size;  /* 링커 스크립트: main 스택 크기 */
    static uint8_t *heapEnd = NULL;

    const uint8_t *maxHeap = &_estack - (uint32_t)&_Min_Stack_Size;
    uint8_t *prev;

    if (heapEnd == NULL) {
        heapEnd = &_end;
    }
    if (heapEnd + incr > maxHeap) {
        errno = ENOMEM;
        return (void *)-1;
    }
    prev = heapEnd;
    heapEnd += incr;
    return prev;
}

#endif /* __GNUC__ */
