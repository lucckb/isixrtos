#include <sys/types.h>
#include <stdlib.h>

int _write (int /*file*/, const void * /*ptr*/, size_t /*len*/)  { return -1; }
int _read (int /*file*/, void * /*ptr*/, size_t /*len*/)  { return -1; }
off_t _lseek (int /*file*/, off_t /*ptr*/, int /*dir*/)  { return -1; }
int _close (int /*file*/)  { return -1; }