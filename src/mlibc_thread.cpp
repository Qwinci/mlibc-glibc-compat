#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include "utils.hpp"

#if __has_include(<hel.h>)
#define MANAGARM 1
#endif

#if MANAGARM
#include <hel.h>
#include <hel-syscalls.h>
#endif
#include <fcntl.h>

struct Guard {
	Guard() {
#if MANAGARM
		int fd = open("/dev/helout", O_WRONLY);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
#endif
	}
} guard {};

pid_t sys_gettid() {
	return gettid();
}

int sys_futex_wait(int *pointer, int expected, const struct timespec *time) {
#if MANAGARM
	if(time) {
		if(helFutexWait(pointer, expected, time->tv_nsec + time->tv_sec * 1000000000))
			return -1;
		return 0;
	}
	if(helFutexWait(pointer, expected, -1))
		return -1;
	return 0;
#else
	return syscall(SYS_futex, pointer, FUTEX_WAIT, expected, time) < 0 ? errno : 0;
#endif
}

int sys_futex_wake(int *pointer) {
#if MANAGARM
	if(helFutexWake(pointer))
		return -1;
	return 0;
#else
	return syscall(SYS_futex, pointer, FUTEX_WAKE, INT_MAX) < 0 ? errno : 0;
#endif
}

struct Mutex {
	unsigned int __mlibc_state;
	unsigned int __mlibc_recursion;
	unsigned int __mlibc_flags;
	int __mlibc_prioceiling;
	int __mlibc_kind;
};
static_assert(sizeof(Mutex) <= 40);

struct Mutexattr {
	int __mlibc_type : 2;
	int __mlibc_robust : 1;
	int __mlibc_protocol : 2;
	int __mlibc_pshared : 1;
	int __mlibc_prioceiling : 1;
};
static_assert(sizeof(Mutexattr) <= 4);

struct Cond {
	unsigned int __mlibc_seq;
	unsigned int __mlibc_flags;
	clockid_t __mlibc_clock;
};
static_assert(sizeof(Cond) <= 48);

struct Condattr {
	int __mlibc_pshared : 1;
	clockid_t __mlibc_clock : 31;
};
static_assert(sizeof(Condattr) <= 4);

#define MLIBC_PTHREAD_MUTEX_TIMED_NP 0
#define MLIBC_PTHREAD_MUTEX_RECURSIVE_NP 1
#define MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP 2
#define MLIBC_PTHREAD_MUTEX_ADAPTIVE_NP 3

static constexpr unsigned int mutex_owner_mask = (static_cast<uint32_t>(1) << 30) - 1;
static constexpr unsigned int mutex_waiters_bit = static_cast<uint32_t>(1) << 31;

EXPORT int __pthread_mutex_init(Mutex *__restrict mutex,
		const Mutexattr *__restrict attr) {
	auto type = attr ? attr->__mlibc_type : 0;
	auto robust = attr ? attr->__mlibc_robust : 0;
	auto protocol = attr ? attr->__mlibc_protocol : 0;
	auto pshared = attr ? attr->__mlibc_pshared : 0;

	mutex->__mlibc_state = 0;
	mutex->__mlibc_recursion = 0;
	mutex->__mlibc_flags = 0;
	mutex->__mlibc_prioceiling = 0; // TODO: We don't implement this.

#if MANAGARM
	if(__builtin_return_address(0) >= (void *)0x40000000) {
		mutex->__mlibc_flags = type << 1;
	} else {
#endif
		mutex->__mlibc_flags |= 1;
		if(type == MLIBC_PTHREAD_MUTEX_RECURSIVE_NP) {
			mutex->__mlibc_kind = MLIBC_PTHREAD_MUTEX_RECURSIVE_NP;
		}else if(type == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP) {
			mutex->__mlibc_kind = MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP;
		}else{
			__ensure(type == MLIBC_PTHREAD_MUTEX_TIMED_NP);
			mutex->__mlibc_kind = 0;
		}
#if MANAGARM
	}
#endif

	// TODO: Other values aren't supported yet.
	__ensure(robust == 0);
	//__ensure(protocol == 0);
	__ensure(pshared == 0);

	return 0;
}
EXPORT_ALIAS("__pthread_mutex_init") int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

EXPORT int __pthread_mutex_destroy(Mutex *mutex) {
	__ensure(!mutex->__mlibc_state);
	return 0;
}
EXPORT_ALIAS("__pthread_mutex_destroy") int pthread_mutex_destroy(pthread_mutex_t *mutex);

EXPORT int __pthread_mutex_lock_internal(Mutex *mutex, bool mlibc) {
	unsigned int this_tid = sys_gettid();
	unsigned int expected = 0;
	while(true) {
		if(!expected) {
			// Try to take the mutex here.
			if(__atomic_compare_exchange_n(&mutex->__mlibc_state,
					&expected, this_tid, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) {
				__ensure(!mutex->__mlibc_recursion);
				mutex->__mlibc_recursion = 1;
				return 0;
			}
		}else{
			// If this (recursive) mutex is already owned by us, increment the recursion level.
			if((expected & mutex_owner_mask) == this_tid) {
				if(mlibc) {
					if((mutex->__mlibc_flags >> 1) != MLIBC_PTHREAD_MUTEX_RECURSIVE_NP) {
						if ((mutex->__mlibc_flags >> 1) == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP)
							return EDEADLK;
						else {
							fputs("mlibc: pthread_mutex deadlock detected!", stderr);
							__ensure(false);
						}
					}
				} else {
					if(mutex->__mlibc_kind != MLIBC_PTHREAD_MUTEX_RECURSIVE_NP) {
						if (mutex->__mlibc_kind == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP)
							return EDEADLK;
						else {
							fputs("mlibc: pthread_mutex deadlock detected!", stderr);
							__ensure(false);
						}
					}
				}
				++mutex->__mlibc_recursion;
				return 0;
			}

			// Wait on the futex if the waiters flag is set.
			if(expected & mutex_waiters_bit) {
				int e = sys_futex_wait((int *)&mutex->__mlibc_state, expected, nullptr);

				// If the wait returns EAGAIN, that means that the mutex_waiters_bit was just unset by
				// some other thread. In this case, we should loop back around.
				if (e && e != EAGAIN) {
					fprintf(stderr, "sys_futex_wait() failed with error code %d\n", e);
				}

				// Opportunistically try to take the lock after we wake up.
				expected = 0;
			}else{
				// Otherwise we have to set the waiters flag first.
				unsigned int desired = expected | mutex_waiters_bit;
				if(__atomic_compare_exchange_n((int *)&mutex->__mlibc_state,
						reinterpret_cast<int*>(&expected), desired, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
					expected = desired;
			}
		}
	}
}

EXPORT int __pthread_mutex_lock(Mutex *mutex) {
#if MANAGARM
	bool mlibc = __builtin_return_address(0) >= (void *)0x40000000;
#else
	bool mlibc = false;
#endif
	return __pthread_mutex_lock_internal(mutex, mlibc);
}
EXPORT_ALIAS("__pthread_mutex_lock") int pthread_mutex_lock(pthread_mutex_t *mutex);

EXPORT int __pthread_mutex_trylock(Mutex *mutex) {
#if MANAGARM
	bool mlibc = __builtin_return_address(0) >= (void *)0x40000000;
#else
	bool mlibc = false;
#endif

	unsigned int this_tid = sys_gettid();
	unsigned int expected = __atomic_load_n(&mutex->__mlibc_state, __ATOMIC_RELAXED);
	if(!expected) {
		// Try to take the mutex here.
		if(__atomic_compare_exchange_n(&mutex->__mlibc_state,
						&expected, this_tid, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) {
			__ensure(!mutex->__mlibc_recursion);
			mutex->__mlibc_recursion = 1;
			return 0;
		}
	} else {
		// If this (recursive) mutex is already owned by us, increment the recursion level.
		if((expected & mutex_owner_mask) == this_tid) {
			if(mlibc) {
				if((mutex->__mlibc_flags >> 1) != MLIBC_PTHREAD_MUTEX_RECURSIVE_NP) {
					return EBUSY;
				}
			} else {
				if(mutex->__mlibc_kind != MLIBC_PTHREAD_MUTEX_RECURSIVE_NP) {
					return EBUSY;
				}
			}
			++mutex->__mlibc_recursion;
			return 0;
		}
	}

	return EBUSY;
}
EXPORT_ALIAS("__pthread_mutex_trylock") int pthread_mutex_trylock(pthread_mutex_t *mutex);

EXPORT int __pthread_mutex_timedlock(Mutex *__restrict,
		const struct timespec *__restrict) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}
EXPORT_ALIAS("__pthread_mutex_timedlock") int pthread_mutex_timedlock(pthread_mutex_t *__restrict, const struct timespec *__restrict);

EXPORT int __pthread_mutex_unlock_internal(Mutex *mutex, bool mlibc) {
	// Decrement the recursion level and unlock if we hit zero.
	__ensure(mutex->__mlibc_recursion);
	if(--mutex->__mlibc_recursion)
		return 0;

	// Reset the mutex to the unlocked state.
	auto state = __atomic_exchange_n(&mutex->__mlibc_state, 0, __ATOMIC_RELEASE);

	// After this point the mutex is unlocked, and therefore we cannot access its contents as it
	// may have been destroyed by another thread.

	unsigned int this_tid = sys_gettid();
	if(mlibc) {
		if ((mutex->__mlibc_flags >> 1) == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP && (state & mutex_owner_mask) != this_tid)
			return EPERM;

		if ((mutex->__mlibc_flags >> 1) == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP && !(state & mutex_owner_mask))
			return EINVAL;
	} else {
		if (mutex->__mlibc_kind == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP && (state & mutex_owner_mask) != this_tid)
			return EPERM;

		if (mutex->__mlibc_kind == MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP && !(state & mutex_owner_mask))
			return EINVAL;
	}

	__ensure((state & mutex_owner_mask) == this_tid);

	if(state & mutex_waiters_bit) {
		// Wake the futex if there were waiters. Since the mutex might not exist at this location
		// anymore, we must conservatively ignore EACCES and EINVAL which may occur as a result.
		int e = sys_futex_wake((int *)&mutex->__mlibc_state);
		__ensure(e >= 0 || e == EACCES || e == EINVAL);
	}

	return 0;
}

EXPORT int __pthread_mutex_unlock(Mutex *mutex) {
#if MANAGARM
	bool mlibc = __builtin_return_address(0) >= (void *)0x40000000;
#else
	bool mlibc = false;
#endif
	return __pthread_mutex_unlock_internal(mutex, mlibc);
}
EXPORT_ALIAS("__pthread_mutex_unlock") int pthread_mutex_unlock(pthread_mutex_t *mutex);

EXPORT int __pthread_mutex_consistent(Mutex *) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}
EXPORT_ALIAS("__pthread_mutex_consistent") int pthread_mutex_consistent(pthread_mutex_t *mutex);

EXPORT int __pthread_mutexattr_init(Mutexattr *attr) {
	attr->__mlibc_type = 0;
	attr->__mlibc_robust = 0;
	attr->__mlibc_pshared = 0;
	attr->__mlibc_protocol = 0;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_init") int pthread_mutexattr_init(pthread_mutexattr_t *attr);

EXPORT int __pthread_mutexattr_destroy(Mutexattr *attr) {
	memset(attr, 0, sizeof(*attr));
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_destroy") int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

EXPORT int __pthread_mutexattr_gettype(const Mutexattr *__restrict attr, int *__restrict type) {
	*type = attr->__mlibc_type;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_gettype") int pthread_mutexattr_gettype(const pthread_mutexattr_t *__restrict attr, int *__restrict type);

EXPORT int __pthread_mutexattr_settype(Mutexattr *attr, int type) {
#if MANAGARM
	if(__builtin_return_address(0) >= (void *)0x40000000) {
		if(type == 1) {
			type = MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP;
		} else if(type == 2) {
			type = MLIBC_PTHREAD_MUTEX_RECURSIVE_NP;
		}
	}
#endif

	if (type != MLIBC_PTHREAD_MUTEX_TIMED_NP && type != MLIBC_PTHREAD_MUTEX_ERRORCHECK_NP
			&& type != MLIBC_PTHREAD_MUTEX_RECURSIVE_NP)
		return EINVAL;

	attr->__mlibc_type = type;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_settype") int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

EXPORT int __pthread_mutexattr_getrobust(const Mutexattr *__restrict attr,
		int *__restrict robust) {
	*robust = attr->__mlibc_robust;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_getrobust") int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust);

EXPORT int __pthread_mutexattr_setrobust(Mutexattr *attr, int robust) {
	if (robust != PTHREAD_MUTEX_STALLED && robust != PTHREAD_MUTEX_ROBUST)
		return EINVAL;

	attr->__mlibc_robust = robust;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_setrobust") int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust);

EXPORT int __pthread_mutexattr_getpshared(const Mutexattr *attr, int *pshared) {
	*pshared = attr->__mlibc_pshared;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_getpshared") int pthread_mutexattr_getpshared(const pthread_mutexattr_t *__restrict attr, int *__restrict pshared);

EXPORT int __pthread_mutexattr_setpshared(Mutexattr *attr, int pshared) {
	if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
		return EINVAL;

	attr->__mlibc_pshared = pshared;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_setpshared") int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);

EXPORT int __pthread_mutexattr_getprotocol(const Mutexattr *__restrict attr,
		int *__restrict protocol) {
	*protocol = attr->__mlibc_protocol;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_getprotocol") int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *__restrict attr, int *__restrict protocol);

EXPORT int __pthread_mutexattr_setprotocol(Mutexattr *attr, int protocol) {
	if (protocol != PTHREAD_PRIO_NONE && protocol != PTHREAD_PRIO_INHERIT
			&& protocol != PTHREAD_PRIO_PROTECT)
		return EINVAL;

	attr->__mlibc_protocol = protocol;
	return 0;
}
EXPORT_ALIAS("__pthread_mutexattr_setprotocol") int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);

EXPORT int __pthread_mutexattr_getprioceiling(const Mutexattr *__restrict attr,
		int *__restrict prioceiling) {
	(void)attr;
	(void)prioceiling;
	return EINVAL;
}
EXPORT_ALIAS("__pthread_mutexattr_getprioceiling") int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *__restrict attr, int *__restrict prioceiling);

EXPORT int __pthread_mutexattr_setprioceiling(Mutexattr *attr, int prioceiling) {
	(void)attr;
	(void)prioceiling;
	return EINVAL;
}
EXPORT_ALIAS("__pthread_mutexattr_setprioceiling") int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);

EXPORT int __pthread_cond_init(Cond *__restrict cond, const Condattr *__restrict attr) {
	auto clock = attr ? attr->__mlibc_clock : CLOCK_REALTIME;
	auto pshared = attr ? attr->__mlibc_pshared : 0;

	cond->__mlibc_clock = clock;
	cond->__mlibc_flags = pshared;

	__atomic_store_n(&cond->__mlibc_seq, 1, __ATOMIC_RELAXED);

	return 0;
}
EXPORT_ALIAS("__pthread_cond_init") int pthread_cond_init(pthread_cond_t *__restrict cond, const pthread_condattr_t *__restrict attr);

EXPORT int __pthread_cond_destroy(Cond *) {
	return 0;
}
EXPORT_ALIAS("__pthread_cond_destroy") int pthread_cond_destroy(pthread_cond_t *cond);

EXPORT int __pthread_cond_broadcast(Cond *cond) {
	__atomic_fetch_add(&cond->__mlibc_seq, 1, __ATOMIC_RELEASE);
	if(int e = sys_futex_wake((int *)&cond->__mlibc_seq); e) {
		fprintf(stderr, "sys_futex_wake() failed with error code %d\n", e);
		__ensure(false);
	}

	return 0;
}
EXPORT_ALIAS("__pthread_cond_broadcast") int pthread_cond_broadcast(pthread_cond_t *cond);

EXPORT int __pthread_cond_timedwait_internal(Cond *__restrict cond, Mutex *__restrict mutex,
		const struct timespec *__restrict abstime, bool mlibc) {
	// TODO: pshared isn't supported yet.
	__ensure(cond->__mlibc_flags == 0);

	constexpr long nanos_per_second = 1'000'000'000;
	if (abstime && (abstime->tv_nsec < 0 || abstime->tv_nsec >= nanos_per_second))
		return EINVAL;

	auto seq = __atomic_load_n(&cond->__mlibc_seq, __ATOMIC_ACQUIRE);

	// TODO: handle locking errors and cancellation properly.
	while (true) {
		if (__pthread_mutex_unlock_internal(mutex, mlibc))
			__ensure(!"Failed to unlock the mutex");

		int e;
		if (abstime) {
			// Adjust for the fact that sys_futex_wait accepts a *timeout*, but
			// pthread_cond_timedwait accepts an *absolute time*.
			// Note: sys_clock_get is available unconditionally.
			struct timespec now;
			if (clock_gettime(cond->__mlibc_clock, &now))
				__ensure(!"clock_gettime() failed");

			struct timespec timeout;
			timeout.tv_sec = abstime->tv_sec - now.tv_sec;
			timeout.tv_nsec = abstime->tv_nsec - now.tv_nsec;

			// Check if abstime has already passed.
			if (timeout.tv_sec < 0 || (timeout.tv_sec == 0 && timeout.tv_nsec < 0)) {
				if (__pthread_mutex_lock_internal(mutex, mlibc))
					__ensure(!"Failed to lock the mutex");
				return ETIMEDOUT;
			} else if (timeout.tv_nsec >= nanos_per_second) {
				timeout.tv_nsec -= nanos_per_second;
				timeout.tv_sec++;
				__ensure(timeout.tv_nsec < nanos_per_second);
			} else if (timeout.tv_nsec < 0) {
				timeout.tv_nsec += nanos_per_second;
				timeout.tv_sec--;
				__ensure(timeout.tv_nsec >= 0);
			}

			e = sys_futex_wait((int *)&cond->__mlibc_seq, seq, &timeout);
		} else {
			e = sys_futex_wait((int *)&cond->__mlibc_seq, seq, nullptr);
		}

		if (__pthread_mutex_lock_internal(mutex, mlibc))
			__ensure(!"Failed to lock the mutex");

		// There are four cases to handle:
		//   1. e == 0: this indicates a (potentially spurious) wakeup. The value of
		//      seq *must* be checked to distinguish these two cases.
		//   2. e == EAGAIN: this indicates that the value of seq changed before we
		//      went to sleep. We don't need to check seq in this case.
		//   3. e == EINTR: a signal was delivered. The man page allows us to choose
		//      whether to go to sleep again or to return 0, but we do the former
		//      to match other libcs.
		//   4. e == ETIMEDOUT: this should only happen if abstime is set.
		if (e == 0) {
			auto cur_seq = __atomic_load_n(&cond->__mlibc_seq, __ATOMIC_ACQUIRE);
			if (cur_seq > seq)
				return 0;
		} else if (e == EAGAIN) {
			__ensure(__atomic_load_n(&cond->__mlibc_seq, __ATOMIC_ACQUIRE) > seq);
			return 0;
		} else if (e == EINTR) {
			continue;
		} else if (e == ETIMEDOUT) {
			__ensure(abstime);
			return ETIMEDOUT;
		} else {
			fprintf(stderr, "sys_futex_wait() failed with error %d\n", e);
			__ensure(false);
		}
	}
}

EXPORT int __pthread_cond_timedwait(Cond *__restrict cond, Mutex *__restrict mutex,
		const struct timespec *__restrict abstime) {
#if MANAGARM
	bool mlibc = __builtin_return_address(0) >= (void *)0x40000000;
#else
	bool mlibc = false;
#endif
	return __pthread_cond_timedwait_internal(cond, mutex, abstime, mlibc);
}
EXPORT_ALIAS("__pthread_cond_timedwait") int pthread_cond_timedwait(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex, const struct timespec *__restrict abstime);

EXPORT int __pthread_cond_wait(Cond *__restrict cond, Mutex *__restrict mutex) {
#if MANAGARM
	bool mlibc = __builtin_return_address(0) >= (void *)0x40000000;
#else
	bool mlibc = false;
#endif
	return __pthread_cond_timedwait_internal(cond, mutex, nullptr, mlibc);
}
EXPORT_ALIAS("__pthread_cond_wait") int pthread_cond_wait(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex);

EXPORT int __pthread_cond_signal(Cond *cond) {
	return __pthread_cond_broadcast(cond);
}
EXPORT_ALIAS("__pthread_cond_signal") int pthread_cond_signal(pthread_cond_t *cond);

EXPORT int __pthread_condattr_init(Condattr *attr) {
	attr->__mlibc_pshared = PTHREAD_PROCESS_PRIVATE;
	attr->__mlibc_clock = CLOCK_REALTIME;
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_init") int pthread_condattr_init(pthread_condattr_t *attr);

EXPORT int __pthread_condattr_destroy(Condattr *attr) {
	memset(attr, 0, sizeof(*attr));
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_destroy") int pthread_condattr_destroy(pthread_condattr_t *attr);

EXPORT int __pthread_condattr_getclock(const Condattr *__restrict attr,
		clockid_t *__restrict clock) {
	*clock = attr->__mlibc_clock;
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_getclock") int pthread_condattr_getclock(const pthread_condattr_t *__restrict attr, clockid_t *__restrict clock);

EXPORT int __pthread_condattr_setclock(Condattr *attr, clockid_t clock) {
	if (clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC
			&& clock != CLOCK_MONOTONIC_RAW && clock != CLOCK_REALTIME_COARSE
			&& clock != CLOCK_MONOTONIC_COARSE && clock != CLOCK_BOOTTIME)
		return EINVAL;

	attr->__mlibc_clock = clock;
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_setclock") int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock);

EXPORT int __pthread_condattr_getpshared(const Condattr *__restrict attr,
		int *__restrict pshared) {
	*pshared = attr->__mlibc_pshared;
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_getpshared") int pthread_condattr_getpshared(const pthread_condattr_t *__restrict attr, int *__restrict pshared);

EXPORT int __pthread_condattr_setpshared(Condattr *attr, int pshared) {
	if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
		return EINVAL;

	attr->__mlibc_pshared = pshared;
	return 0;
}
EXPORT_ALIAS("__pthread_condattr_setpshared") int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);
