#include "utils.hpp"
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>

template<typename T>
static inline T* load(const char* name) {
	return reinterpret_cast<T*>(dlsym(RTLD_NEXT, name));
}

namespace real {
	auto attr_init = load<decltype(pthread_attr_init)>("pthread_attr_init");
	auto attr_destroy = load<decltype(pthread_attr_destroy)>("pthread_attr_destroy");
	auto attr_getdetachstate = load<decltype(pthread_attr_getdetachstate)>("pthread_attr_getdetachstate");
	auto attr_setdetachstate = load<decltype(pthread_attr_setdetachstate)>("pthread_attr_setdetachstate");
	auto attr_getstacksize = load<decltype(pthread_attr_getstacksize)>("pthread_attr_getstacksize");
	auto attr_setstacksize = load<decltype(pthread_attr_setstacksize)>("pthread_attr_setstacksize");
	auto attr_getstackaddr = load<decltype(pthread_attr_getstackaddr)>("pthread_attr_getstackaddr");
	auto attr_setstackaddr = load<decltype(pthread_attr_setstackaddr)>("pthread_attr_setstackaddr");
	auto attr_getstack = load<decltype(pthread_attr_getstack)>("pthread_attr_getstack");
	auto attr_setstack = load<decltype(pthread_attr_setstack)>("pthread_attr_setstack");
	auto attr_getguardsize = load<decltype(pthread_attr_getguardsize)>("pthread_attr_getguardsize");
	auto attr_setguardsize = load<decltype(pthread_attr_setguardsize)>("pthread_attr_setguardsize");
	auto attr_getscope = load<decltype(pthread_attr_getscope)>("pthread_attr_getscope");
	auto attr_setscope = load<decltype(pthread_attr_setscope)>("pthread_attr_setscope");
	auto attr_getschedparam = load<decltype(pthread_attr_getschedparam)>("pthread_attr_getschedparam");
	auto attr_setschedparam = load<decltype(pthread_attr_setschedparam)>("pthread_attr_setschedparam");
	auto attr_getschedpolicy = load<decltype(pthread_attr_getschedpolicy)>("pthread_attr_getschedpolicy");
	auto attr_setschedpolicy = load<decltype(pthread_attr_setschedpolicy)>("pthread_attr_setschedpolicy");
	auto attr_getinheritsched = load<decltype(pthread_attr_getinheritsched)>("pthread_attr_getinheritsched");
	auto attr_setinheritsched = load<decltype(pthread_attr_setinheritsched)>("pthread_attr_setinheritsched");
	#if __MLIBC_LINUX_OPTION
	auto attr_getaffinity_np = load<decltype(pthread_attr_getaffinity_np)>("pthread_attr_getaffinity_np");
	auto attr_setaffinity_np = load<decltype(pthread_attr_setaffinity_np)>("pthread_attr_setaffinity_np");
	auto attr_getsigmask_np = load<decltype(pthread_attr_getsigmask_np)>("pthread_attr_getsigmask_np");
	auto attr_setsigmask_np = load<decltype(pthread_attr_setsigmask_np)>("pthread_attr_setsigmask_np");
	auto getattr_np = load<decltype(pthread_getattr_np)>("pthread_getattr_np");;
	auto getaffinity_np = load<decltype(pthread_getaffinity_np)>("pthread_getaffinity_np");
	auto setaffinity_np = load<decltype(pthread_setaffinity_np)>("pthread_setaffinity_np");
	#endif

	auto thread_create = load<decltype(pthread_create)>("pthread_create");
}

enum class ThreadDetachState {
	Joinable,
	Detached
};

enum class MutexType {
	TimedNp,
	RecursiveNp,
	ErrorCheckNp,
	AdaptiveNp
};

enum class MutexRobust {
	Stalled,
	Robust
};

enum class MutexPriority {
	None,
	Inherit,
	Protect
};

enum class RwLockPrefer {
	ReaderNp,
	WriterNp,
	WriterNonrecursiveNp
};

enum class InheritSched {
	Inherit,
	Explicit
};

enum class Scope {
	System,
	Process
};

enum class Visitibility {
	Private,
	Shared
};

enum class CancelEnable {
	Enable,
	Disable
};

enum class CancelType {
	Deferred,
	Asynchronous
};

struct wrap_attr_t {
	void* stack;
	size_t stack_size;
	size_t guard_size;
	cpu_set_t* cpu_set;
	sched_param param;
	ThreadDetachState detach_state : 1;
	Scope scope : 1;
	InheritSched inherit_sched : 1;
	int sched_policy;
};
static_assert(sizeof(wrap_attr_t) <= 56);

EXPORT int __pthread_attr_init(wrap_attr_t *attr) {
	*attr = {};
	attr->stack_size = 0x200000;
	attr->guard_size = 4096;
	return 0;
}
EXPORT int __pthread_attr_destroy(wrap_attr_t *attr) {
	if(attr->cpu_set)
		free(attr->cpu_set);
	return 0;
}

EXPORT int __pthread_attr_getdetachstate(const wrap_attr_t *attr, int *state) {
	*state = static_cast<int>(attr->detach_state);
	return 0;
}
EXPORT int __pthread_attr_setdetachstate(wrap_attr_t *attr, int state) {
	attr->detach_state = static_cast<ThreadDetachState>(state);
	return 0;
}

EXPORT int __pthread_attr_getstacksize(const wrap_attr_t *__restrict attr, size_t *__restrict size) {
	*size = attr->stack_size;
	return 0;
}
EXPORT int __pthread_attr_setstacksize(wrap_attr_t *attr, size_t size) {
	attr->stack_size = size;
	return 0;
}

EXPORT int __pthread_attr_getstackaddr(const wrap_attr_t *attr, void **stack) {
	*stack = attr->stack;
	return 0;
}
EXPORT int __pthread_attr_setstackaddr(wrap_attr_t *attr, void *stack) {
	attr->stack = stack;
	return 0;
}

EXPORT int __pthread_attr_getstack(const wrap_attr_t *attr, void **stack, size_t* size) {
	*stack = attr->stack;
	*size = attr->stack_size;
	return 0;
}
EXPORT int __pthread_attr_setstack(wrap_attr_t *attr, void *stack, size_t size) {
	attr->stack = stack;
	attr->stack_size = size;
	return 0;
}

EXPORT int __pthread_attr_getguardsize(const wrap_attr_t *__restrict attr, size_t *__restrict size) {
	*size = attr->guard_size;
	return 0;
}
EXPORT int __pthread_attr_setguardsize(wrap_attr_t *attr, size_t size) {
	attr->guard_size = size;
	return 0;
}

EXPORT int __pthread_attr_getscope(const wrap_attr_t *attr, int* scope) {
	*scope = static_cast<int>(attr->scope);
	return 0;
}
EXPORT int __pthread_attr_setscope(wrap_attr_t *attr, int scope) {
	attr->scope = static_cast<Scope>(scope);
	return 0;
}

EXPORT int __pthread_attr_getschedparam(const wrap_attr_t *__restrict attr, struct sched_param *__restrict param) {
	*param = attr->param;
	return 0;
}
EXPORT int __pthread_attr_setschedparam(wrap_attr_t *__restrict attr, const struct sched_param *__restrict param) {
	attr->param = *param;
	return 0;
}

EXPORT int __pthread_attr_getschedpolicy(const wrap_attr_t *__restrict attr, int *__restrict policy) {
	*policy = attr->sched_policy;
	return 0;
}
EXPORT int __pthread_attr_setschedpolicy(wrap_attr_t *__restrict attr, int policy) {
	attr->sched_policy = policy;
	return 0;
}

EXPORT int __pthread_attr_getinheritsched(const wrap_attr_t *__restrict attr, int *__restrict inherit) {
	*inherit = static_cast<int>(attr->inherit_sched);
	return 0;
}
EXPORT int __pthread_attr_setinheritsched(wrap_attr_t *__restrict attr, int inherit) {
	attr->inherit_sched = static_cast<InheritSched>(inherit);
	return 0;
}
#if __MLIBC_LINUX_OPTION
EXPORT int __pthread_attr_getaffinity_np(const wrap_attr_t *__restrict attr, size_t size, cpu_set_t *__restrict set) {
	__ensure(size == sizeof(cpu_set_t));
	if(attr->cpu_set)
		*set = *attr->cpu_set;
	else
		*set = {};
	return 0;
}
EXPORT int __pthread_attr_setaffinity_np(wrap_attr_t *__restrict attr, size_t size, const cpu_set_t *__restrict set) {
	__ensure(size == sizeof(cpu_set_t));
	if(!attr->cpu_set) {
		attr->cpu_set = static_cast<cpu_set_t*>(malloc(sizeof(cpu_set_t)));
		__ensure(attr->cpu_set);
	}
	*attr->cpu_set = *set;
	return 0;
}

EXPORT int __pthread_attr_getsigmask_np(const wrap_attr_t *__restrict, sigset_t *__restrict) {
	__ensure(!"Not implemented");
}
EXPORT int __pthread_attr_setsigmask_np(wrap_attr_t *__restrict, const sigset_t *__restrict) {
	__ensure(!"Not implemented");
}

EXPORT int __pthread_getattr_np(pthread_t thread, wrap_attr_t *attr) {
	__ensure(!"Not implemented");
}
#endif

EXPORT_ALIAS("__pthread_attr_init") int pthread_attr_init(pthread_attr_t *);
EXPORT_ALIAS("__pthread_attr_destroy") int pthread_attr_destroy(pthread_attr_t *);
EXPORT_ALIAS("__pthread_attr_getdetachstate") int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
EXPORT_ALIAS("__pthread_attr_setdetachstate") int pthread_attr_setdetachstate(pthread_attr_t *, int);
EXPORT_ALIAS("__pthread_attr_getstacksize") int pthread_attr_getstacksize(const pthread_attr_t *__restrict, size_t *__restrict);
EXPORT_ALIAS("__pthread_attr_setstacksize") int pthread_attr_setstacksize(pthread_attr_t *, size_t);
EXPORT_ALIAS("__pthread_attr_getstackaddr") int pthread_attr_getstackaddr(const pthread_attr_t *, void **);
EXPORT_ALIAS("__pthread_attr_setstackaddr") int pthread_attr_setstackaddr(pthread_attr_t *, void *);
EXPORT_ALIAS("__pthread_attr_getstack") int pthread_attr_getstack(const pthread_attr_t *, void **, size_t*);
EXPORT_ALIAS("__pthread_attr_setstack") int pthread_attr_setstack(pthread_attr_t *, void *, size_t);
EXPORT_ALIAS("__pthread_attr_getguardsize") int pthread_attr_getguardsize(const pthread_attr_t *__restrict, size_t *__restrict);
EXPORT_ALIAS("__pthread_attr_setguardsize") int pthread_attr_setguardsize(pthread_attr_t *, size_t);
EXPORT_ALIAS("__pthread_attr_getscope") int pthread_attr_getscope(const pthread_attr_t *, int*);
EXPORT_ALIAS("__pthread_attr_setscope") int pthread_attr_setscope(pthread_attr_t *, int);
EXPORT_ALIAS("__pthread_attr_getschedparam") int pthread_attr_getschedparam(const pthread_attr_t *__restrict, struct sched_param *__restrict);
EXPORT_ALIAS("__pthread_attr_setschedparam") int pthread_attr_setschedparam(pthread_attr_t *__restrict, const struct sched_param *__restrict);
EXPORT_ALIAS("__pthread_attr_getschedpolicy") int pthread_attr_getschedpolicy(const pthread_attr_t *__restrict, int *__restrict);
EXPORT_ALIAS("__pthread_attr_setschedpolicy") int pthread_attr_setschedpolicy(pthread_attr_t *__restrict, int);
EXPORT_ALIAS("__pthread_attr_getinheritsched") int pthread_attr_getinheritsched(const pthread_attr_t *__restrict, int *__restrict);
EXPORT_ALIAS("__pthread_attr_setinheritsched") int pthread_attr_setinheritsched(pthread_attr_t *__restrict, int);
#if __MLIBC_LINUX_OPTION
EXPORT_ALIAS("__pthread_attr_getaffinity_np") int pthread_attr_getaffinity_np(const pthread_attr_t *__restrict, size_t, cpu_set_t *__restrict);
EXPORT_ALIAS("__pthread_attr_setaffinity_np") int pthread_attr_setaffinity_np(pthread_attr_t *__restrict, size_t, const cpu_set_t *__restrict);
EXPORT_ALIAS("__pthread_attr_getsigmask_np") int pthread_attr_getsigmask_np(const pthread_attr_t *__restrict, sigset_t *__restrict);
EXPORT_ALIAS("__pthread_attr_setsigmask_np") int pthread_attr_setsigmask_np(pthread_attr_t *__restrict, const sigset_t *__restrict);
EXPORT_ALIAS("__pthread_getattr_np") int pthread_getattr_np(pthread_t, pthread_attr_t *);
#endif

EXPORT int __pthread_create(pthread_t *__restrict thread, const wrap_attr_t *__restrict attr, void *(*fn)(void *), void* __restrict arg) {
	pthread_attr_t real_attr;
	int res;
	if(attr) {
		real::attr_init(&real_attr);
		real::attr_setstack(&real_attr, attr->stack, attr->stack_size);
		real::attr_setguardsize(&real_attr, attr->guard_size);
#if __MLIBC_LINUX_OPTION
		real::attr_setaffinity_np(&real_attr, sizeof(cpu_set_t), attr->cpu_set);
#endif
		real::attr_setschedparam(&real_attr, &attr->param);
		real::attr_setdetachstate(&real_attr, attr->detach_state == ThreadDetachState::Joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);
		real::attr_setscope(&real_attr, attr->scope == Scope::System ? PTHREAD_SCOPE_SYSTEM : PTHREAD_SCOPE_PROCESS);
		real::attr_setinheritsched(&real_attr, attr->inherit_sched == InheritSched::Inherit ? PTHREAD_INHERIT_SCHED : PTHREAD_EXPLICIT_SCHED);
		real::attr_setschedpolicy(&real_attr, attr->sched_policy);

		res = real::thread_create(thread, &real_attr, fn, arg);

		real::attr_destroy(&real_attr);
	} else {
		res = real::thread_create(thread, nullptr, fn, arg);
	}

	return res;
}

EXPORT_ALIAS("__pthread_create") int pthread_create(pthread_t *__restrict thread, const pthread_attr_t *__restrict attr, void *(*fn)(void *), void* __restrict arg);

// This is a function that glibc exports
EXPORT int __pthread_key_create(pthread_key_t *out, void (*destructor)(void *)) {
	return pthread_key_create(out, destructor);
}
