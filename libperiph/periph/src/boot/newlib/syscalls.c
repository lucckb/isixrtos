#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdnoreturn.h>

int _write(int /*file*/, const void * /*ptr*/, size_t /*len*/)  { return -1; }
int _read(int /*file*/, void * /*ptr*/, size_t /*len*/)  { return -1; }
off_t _lseek(int /*file*/, off_t /*ptr*/, int /*dir*/)  { return -1; }
int _close(int /*file*/)  { return -1; }
int _fstat(int /*file*/, struct stat* /*buf*/) { return -1; }
int _isatty(int /*file*/) { return 0; }

noreturn void _exit(int /*status*/) { for (;;); };

pid_t _getpid(void) { return -1; }
int _kill(pid_t /*pid*/, int /*sig*/) { return -1; }

void* _sbrk(intptr_t /*increment*/) { return NULL; }
