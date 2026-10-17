/*
 * syscalls.c - newlib 시스템 콜 최소 구현 (파일/콘솔 입출력 없음)
 * vsnprintf 같은 문자열 함수만 쓰므로 실제로 호출되지는 않는다.
 * printf 를 UART 로 내보내고 싶다면 _write() 를 구현하면 된다.
 */
/* GCC(newlib) 전용. IAR 은 자체 라이브러리(DLIB)를 쓰므로 컴파일되지 않음 */
#if defined(__GNUC__) && !defined(__ICCARM__)

#include <errno.h>
#include <sys/stat.h>

int _close(int fd);
int _lseek(int fd, int ptr, int dir);
int _read(int fd, char *p, int len);
int _write(int fd, char *p, int len);
int _isatty(int fd);
int _fstat(int fd, struct stat *st);

int _close(int fd)                    { (void)fd; return -1; }
int _lseek(int fd, int ptr, int dir)  { (void)fd; (void)ptr; (void)dir; return 0; }
int _read(int fd, char *p, int len)   { (void)fd; (void)p; (void)len; return 0; }
int _write(int fd, char *p, int len)  { (void)fd; (void)p; return len; }
int _isatty(int fd)                   { (void)fd; return 1; }
int _fstat(int fd, struct stat *st)   { (void)fd; st->st_mode = S_IFCHR; return 0; }

#endif /* __GNUC__ */
