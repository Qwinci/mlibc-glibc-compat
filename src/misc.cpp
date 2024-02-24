#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sched.h>
#include <locale.h>
#include <iconv.h>
#include <dlfcn.h>
#include <math.h>
#include <sys/sysinfo.h>
#include "utils.hpp"

#if __has_include(<hel.h>)
#define MANAGARM 1
#endif

struct mallinfo2 {
	size_t arena;
	size_t ordblks;
	size_t smblks;
	size_t hblks;
	size_t hblkhd;
	size_t usmblks;
	size_t fsmblks;
	size_t uordblks;
	size_t fordblks;
	size_t keepcost;
};

EXPORT mallinfo2 mallinfo2() {
	return {};
}

EXPORT int malloc_trim(size_t pad) {
	return 0;
}

EXPORT void *__rawmemchr(const void *s, int c) {
	return const_cast<void *>(memchr(s, c, SIZE_MAX));
}

EXPORT_ALIAS("__rawmemchr") void *rawmemchr(const void *s, int c);

EXPORT char *__xpg_strerror_r(int num, char *buf, size_t size) {
	strerror_r(num, buf, size);
	return buf;
}

extern "C" uintptr_t *__dlapi_entrystack();

extern char **environ;

EXPORT int __libc_start_main(int (*fn)(int, char **argv, char **envp), int, char **, void (*fini)(), void (*rtdl_fini)(), void *stack_end) {
	auto stack = __dlapi_entrystack();
	
	int argc = static_cast<int>(*stack);
	auto argv = reinterpret_cast<char **>(stack + 1);

	auto result = fn(argc, argv, environ);
	exit(result);
}

asm(R"(
.globl _setjmp
_setjmp:
jmp setjmp
.globl _longjmp
_longjmp:
jmp _longjmp
)");

#if MANAGARM
#define WRAP_SYSCALL 1
#else
#define WRAP_SYSCALL 0
#endif

#if WRAP_SYSCALL
EXPORT long syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5) {
	fprintf(stderr, "syscall %ld\n", num);
	__ensure(!"syscall is not implemented");
}
#endif

#if !WRAP_SYSCALL
asm(R"(
.globl syscall
.type syscall, @function
syscall:
mov %rdi, %rax
mov %rsi, %rdi
mov %rdx, %rsi
mov %rcx, %rdx
mov %r8, %r10
mov %r9, %r8
mov 8(%rsp), %r9
syscall
cmpq $-4095, %rax
jae 1f
ret
1:
mov __mlibc_errno@GOTTPOFF(%rip), %rdi
neg %eax
mov %eax, %fs:0(%rdi)
mov $-1, %eax
ret
)");
#endif

// todo upstream this
EXPORT int get_nprocs() {
	// todo real nprocs
	return 1;
}

EXPORT int eaccess(const char *path, int mode) {
	fputs("eaccess is not implemented properly", stderr);
	return access(path, mode);
}

EXPORT int __libc_current_sigrtmin() {
	return 34;
}

EXPORT int __libc_current_sigrtmax() {
	return 64;
}

EXPORT int close_range(unsigned int first, unsigned int last, unsigned int flags) {
	// todo implement flags
	for (unsigned int i = first; i <= last; ++i) {
		close(i);
	}
	return 0;
}

EXPORT int statx(int fd, const char *__restrict path, int flags, unsigned int mask, struct statx *__restrict buf) {
	errno = ENOSYS;
	return -1;
}

EXPORT int epoll_pwait2(int fd, epoll_event *events, int n, const timespec *timeout,
	const sigset_t *sigmask) {
	int timeout_ms = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000L;
	return epoll_pwait(fd, events, n, timeout_ms, sigmask);
}

// todo upstream
EXPORT int res_search(const char *dname, int _class, int type, unsigned char *answer, int answer_size) {
	errno = ENOSYS;
	return -1;
}

EXPORT int __sched_cpucount(size_t set_size, const cpu_set_t *set) {
	int count = 0;
	for(size_t i = 0; i < set_size / __NCPUBITS; ++i) {
		count += __builtin_popcountl(set->__bits[i]);
	}
	return count;
}

// todo fix in mlibc
EXPORT char *gettext(const char *msgid) {
	return const_cast<char *>(msgid);
}

struct __locale_struct {
	struct __locale_data *locales[13];
	const unsigned short *ctype_b;
	const int32_t *ctype_tolower;
	const int32_t *ctype_toupper;
	const char *names[13];
};

extern "C" int32_t **__ctype_tolower_loc();
extern "C" int32_t **__ctype_toupper_loc();
extern "C" const unsigned short **__ctype_b_loc();

// todo fix in mlibc
EXPORT locale_t newlocale(int, const char *, locale_t) {
	auto *locale = static_cast<__locale_struct *>(malloc(sizeof(__locale_struct)));
	if(!locale)
		return nullptr;
	locale->ctype_tolower = *__ctype_tolower_loc();
	locale->ctype_toupper = *__ctype_toupper_loc();
	locale->ctype_b = *__ctype_b_loc();
	return locale;
}

EXPORT void freelocale(locale_t locale) {
	free(locale);
}

EXPORT locale_t duplocale(locale_t old) {
	auto *locale = static_cast<__locale_struct *>(malloc(sizeof(__locale_struct)));
	if(!locale)
		return nullptr;
	*locale = *static_cast<__locale_struct *>(old);
	return locale;
}

#define L_LC_NUMERIC 1
#define L_LC_TIME 2
#define L_LC_COLLATE 3
#define L_LC_MONETARY 4
#define L_LC_MESSAGES 5
#define L_LC_ALL 6

EXPORT char *setlocale(int category, const char *locale) {
	return const_cast<char *>("C");
}

EXPORT int initstate_r(unsigned int seed, char *state, size_t n, struct random_data *buf) {
	return 0;
}

EXPORT int random_r(random_data *buf, int32_t *result) {
	return 1;
}

EXPORT void arc4random_buf(void *buf, size_t n) {

}

EXPORT uint32_t arc4random() {
	__ensure(false);
	return 0;
}

// todo implement these
EXPORT int strfromf128(char *__restrict s, size_t n, const char *__restrict format, __float128 fp) {
	__ensure(false);
	return ENOSYS;
}

EXPORT __float128 strtof128(const char *__restrict str, char **__restrict end) {
	if(end)
		*end = const_cast<char *>(str);
	__ensure(false);
}

// todo implement in mlibc
EXPORT char *bindtextdomain(const char *domainname, const char *dirname) {
	return const_cast<char *>("UTF-8");
}

// todo implement in mlibc
EXPORT char *bind_textdomain_codeset(const char *domainname, const char *codeset) {
	return const_cast<char *>("UTF-8");
}

#undef iconv_open
#undef iconv_close
#undef iconv

// todo implement in mlibc
EXPORT iconv_t iconv_open(const char *tocode, const char *fromcode) {
	return reinterpret_cast<iconv_t>(1);
}

EXPORT size_t iconv(iconv_t cd, char **__restrict inbuf, size_t *__restrict inbytesleft, char **__restrict outbuf, size_t *__restrict outbytesleft) {
	(void)inbytesleft;
	(void)outbytesleft;

	if(cd == (iconv_t)1) { // UTF-8 to UTF-8
		memcpy(inbuf, outbuf, sizeof(inbuf));
		return sizeof(outbuf);
	}
	__ensure(!"iconv() not implemented");
	__builtin_unreachable();
}

EXPORT int iconv_close(iconv_t) {
	return 0;
}

// todo implement in mlibc
EXPORT char *dcgettext(const char *domainname, const char *msgid) {
	return const_cast<char *>(msgid);
}

static auto fmodl_real = reinterpret_cast<long double (*)(long double, long double)>(
	dlsym(RTLD_NEXT, "fmodl")
);

EXPORT long double fmodl(long double x, long double y) {
	// todo figure out why this is needed
	if(isnan(x))
		return 0.0;
	return fmodl_real(x, y);
}

extern "C" [[noreturn]] void __ensure_fail_wrap(const char *assertion, const char *file, unsigned int line,
		const char *function) {
	fprintf(stderr, "In function %s, file %s:%u\n__ensure(%s) failed\n", function, file, line, assertion);
	quick_exit(1);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

EXPORT long sysconf(int name) {
	switch(name) {
	// _SC_NGROUPS_MAX
	case 3:
		return 65536;
	// _SC_TZNAME_MAX
	case 6:
		return 6;
	// _SC_MEMLOCK
	case 17:
		return 200809L;
	// _SC_PAGE_SIZE
	case 30:
		return 0x1000;
	// _SC_PHYS_PAGES
	case 85: {
		return 262144;
	}
	default:
		fprintf(stderr, "sysconf unimplemented: %d\n", name);
		return 0;
	}
}

#include <sys/syscall.h>

EXPORT ssize_t sendfile(int out, int in, off_t *offset, size_t count) {
#if MANAGARM
	return count;
#else
	return syscall(SYS_sendfile, out, in, offset, count);
#endif
}

EXPORT char __libc_single_threaded = 0;
