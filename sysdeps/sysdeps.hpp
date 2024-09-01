#pragma once
#include <time.h>

namespace mlibc_glibc_compat {

int sys_futex_wait(int *pointer, int expected, const timespec *time);
int sys_futex_wake(int *pointer);

}
