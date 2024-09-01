#include "sysdeps.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <hel.h>
#include <hel-syscalls.h>

namespace {

struct Guard {
	Guard() {
		int fd = open("/dev/helout", O_WRONLY);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
	}
};

Guard guard{};

}

namespace mlibc_glibc_compat {

int sys_futex_wait(int *pointer, int expected, const timespec *time) {
	if(time) {
		if(helFutexWait(pointer, expected, time->tv_nsec + time->tv_sec * 1000000000))
			return -1;
		return 0;
	}
	if(helFutexWait(pointer, expected, -1))
		return -1;
	return 0;
}

int sys_futex_wake(int *pointer) {
	if(helFutexWake(pointer))
		return -1;
	return 0;
}

}

