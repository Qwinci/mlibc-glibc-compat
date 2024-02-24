#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include <wctype.h>
#include <time.h>
#include <pthread.h>
#include <resolv.h>
#include <fcntl.h>
#include "utils.hpp"

EXPORT int __isoc99_fscanf(FILE *__restrict file, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vfscanf(file, format, ap);
	va_end(ap);
	return result;
}

EXPORT int __isoc99_vfscanf(FILE *__restrict file, const char *__restrict format, va_list ap) {
	return vfscanf(file, format, ap);
}

EXPORT int __isoc99_scanf(const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vscanf(format, ap);
	va_end(ap);
	return result;
}

EXPORT int __isoc99_vsscanf(const char *__restrict str, const char *__restrict format, va_list ap) {
	return vsscanf(str, format, ap);
}

EXPORT int __isoc99_sscanf(const char *__restrict str, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vsscanf(str, format, ap);
	va_end(ap);
	return result;
}

EXPORT ssize_t __getdelim(char **line, size_t *n, int delim, FILE *file) {
	return getdelim(line, n, delim, file);
}

EXPORT long __isoc99_strtol(const char *__restrict str, char **__restrict end, int base) {
	return strtol(str, end, base);
}

EXPORT long long __isoc99_strtoll(const char *__restrict str, char **__restrict end, int base) {
	return strtoll(str, end, base);
}

EXPORT unsigned long __isoc99_strtoul(const char *__restrict str, char **__restrict end, int base) {
	return strtoul(str, end, base);
}

EXPORT unsigned long long __isoc99_strtoull(const char *__restrict str, char **__restrict end, int base) {
	return strtoull(str, end, base);
}

EXPORT long __isoc99_wcstol(const wchar_t *__restrict ptr, wchar_t **__restrict end, int base) {
	return wcstol(ptr, end, base);
}

EXPORT char *__strtok_r(char *__restrict s, const char *__restrict delim, char **__restrict m) {
	return strtok_r(s, delim, m);
}

EXPORT char *__xpg_basename(const char *path) {
	return __mlibc_gnu_basename_c(path);
}

extern "C" int __cxa_atexit(void (*fn)(void *), void *arg, void *dso_handle);

EXPORT int __cxa_thread_atexit_impl(void (*fn)(void *), void *arg, void *dso_handle) {
	return __cxa_atexit(fn, arg, dso_handle);
}

EXPORT locale_t __uselocale(locale_t locale) {
	return uselocale(locale);
}

EXPORT char *__nl_langinfo_l(nl_item item, locale_t locale) {
	return nl_langinfo_l(item, locale);
}

EXPORT wctype_t __wctype_l(const char *name, locale_t locale) {
	// todo this has wrong signature in mlibc
	//return wctype_l(name);
	return wctype(name);
}

EXPORT wint_t __towupper_l(wint_t value, locale_t locale) {
	return towupper_l(value, locale);
}

EXPORT wint_t __towlower_l(wint_t value, locale_t locale) {
	return towlower_l(value, locale);
}

EXPORT wint_t __iswctype_l(wint_t value, wctype_t desc, locale_t locale) {
	// todo this has wrong signature in mlibc
	//return iswctype_l(value, desc);
	return iswctype(value, desc);
}

EXPORT float __strtof_l(const char *__restrict str, char **__restrict end, locale_t locale) {
	return strtof_l(str, end, locale);
}

EXPORT double __strtod_l(const char *__restrict str, char **__restrict end, locale_t locale) {
	return strtod_l(str, end, locale);
}

EXPORT int __strcoll_l(const char *s1, const char *s2, locale_t locale) {
	return strcoll_l(s1, s2, locale);
}

EXPORT int __wcscoll_l(const wchar_t *ws1, const wchar_t *ws2, locale_t locale) {
	// todo this ignores locale, implement in mlibc the locale version
	return wcscoll(ws1, ws2);
}

EXPORT size_t __wcsxfrm_l(wchar_t *__restrict ws1, const wchar_t *__restrict ws2, size_t max_size, locale_t locale) {
	// todo this ignores locale, implement in mlibc the locale version
	return wcsxfrm(ws1, ws2, max_size);
}

EXPORT size_t __wcsftime_l(wchar_t *__restrict dest, size_t max_size, const wchar_t *__restrict format, const tm *__restrict value, locale_t locale) {
	// todo this ignores locale, implement in mlibc the locale version
	return wcsftime(dest, max_size, format, value);
}

EXPORT size_t __strxfrm_l(char *__restrict dest, const char *__restrict src, size_t max_size, locale_t locale) {
	// todo this ignores locale, implement in mlibc the locale version
	return strxfrm(dest, src, max_size);
}

EXPORT size_t __strftime_l(char *__restrict dest, size_t max_size, const char *__restrict format, const tm *__restrict value, locale_t locale) {
	// todo this ignores locale, implement in mlibc the locale version
	return strftime(dest, max_size, format, value);
}

EXPORT locale_t __newlocale(int category, const char *locale, locale_t base) {
	return newlocale(category, locale, base);
}

EXPORT void __freelocale(locale_t locale) {
	freelocale(locale);
}

EXPORT locale_t __duplocale(locale_t locale) {
	return duplocale(locale);
}

EXPORT int __register_atfork(void (*prepare)(), void (*parent)(), void (*child)(), void *dso_handle) {
	return pthread_atfork(prepare, parent, child);
}

EXPORT int __res_ninit(res_state state) {
	return res_ninit(state);
}

EXPORT void __res_nclose(res_state state) {
	res_nclose(state);
}

EXPORT int __dn_expand(const unsigned char *msg, const unsigned char *eom_orig, const unsigned char *comp_dn, char *exp_dn, int size) {
	return dn_expand(msg, eom_orig, comp_dn, exp_dn, size);
}

EXPORT int __res_nquery(res_state state, const char *name, int _class, int type, unsigned char *answer, int answer_size) {
	// todo implement in mlibc
	return -1;
	//return res_nquery(state, name, _class, type, answer, answer_size);
}

EXPORT char *__strdup(const char *str) {
	return strdup(str);
}

asm(R"(
.globl open64
.globl __open_2
.globl __open64_2
open64:
__open_2:
__open64_2:
	jmp open

.globl fcntl64
fcntl64:
	jmp fcntl

.globl openat64
.globl __openat_2
.globl __openat64_2
openat64:
__openat_2:
__openat64_2:
	jmp openat
)");

long __timezone;
EXPORT_ALIAS("__timezone") long timezone;

EXPORT_ALIAS("__isoc99_fscanf") int __isoc23_fscanf(FILE *__restrict file, const char *__restrict format, ...);
EXPORT_ALIAS("__isoc99_vfscanf") int __isoc23_vfscanf(FILE *__restrict file, const char *__restrict format, va_list ap);
EXPORT_ALIAS("__isoc99_scanf") int __isoc23_scanf(const char *__restrict format, ...);
EXPORT_ALIAS("__isoc99_vsscanf") int __isoc23_vsscanf(const char *__restrict str, const char *__restrict format, va_list ap);
EXPORT_ALIAS("__isoc99_sscanf") int __isoc23_sscanf(const char *__restrict str, const char *__restrict format, ...);
EXPORT_ALIAS("__isoc99_strtol") long __isoc23_strtol(const char *__restrict str, char **__restrict end, int base);
EXPORT_ALIAS("__isoc99_strtoll") long long __isoc23_strtoll(const char *__restrict str, char **__restrict end, int base);
EXPORT_ALIAS("__isoc99_strtoul") unsigned long __isoc23_strtoul(const char *__restrict str, char **__restrict end, int base);
EXPORT_ALIAS("__isoc99_strtoull") unsigned long long __isoc23_strtoull(const char *__restrict str, char **__restrict end, int base);
EXPORT_ALIAS("__isoc99_wcstol") long __isoc23_wcstol(const wchar_t *__restrict ptr, wchar_t **__restrict end, int base);
