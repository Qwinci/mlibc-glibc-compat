#include "sysdeps.hpp"
#include <sys/syscall.h>
#include <linux/futex.h>

namespace mlibc_glibc_compat {

int sys_futex_wait(int *pointer, int expected, const struct timespec *time) {
	return syscall(SYS_futex, pointer, FUTEX_WAIT, expected, time) < 0 ? errno : 0;
}

int sys_futex_wake(int *pointer) {
	return syscall(SYS_futex, pointer, FUTEX_WAKE, INT_MAX) < 0 ? errno : 0;	
}

}
