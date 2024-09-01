#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <sys/select.h>
#include <unistd.h>
#include <poll.h>
#include <syslog.h>
#include <stdlib.h>
#include "utils.hpp"

EXPORT int __printf_chk(int flag, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vprintf(format, ap);
	va_end(ap);
	return result;
}

EXPORT int __fprintf_chk(FILE *__restrict file, int flag, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vfprintf(file, format, ap);
	va_end(ap);
	return result;
}

EXPORT int __vfprintf_chk(FILE *__restrict file, int flag, const char *__restrict format, va_list ap) {
	return vfprintf(file, format, ap);;
}

EXPORT int __sprintf_chk(char *__restrict dest, int flag, size_t real_size, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vsprintf(dest, format, ap);
	va_end(ap);
	return result;
}

EXPORT int __snprintf_chk(char *__restrict dest, size_t max_size, int flag, size_t real_size, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vsnprintf(dest, max_size, format, ap);
	va_end(ap);
	return result;
}

EXPORT int __vsnprintf_chk(char *__restrict dest, size_t max_size, int flag, size_t real_size, const char *__restrict format, va_list ap) {
	return vsnprintf(dest, max_size, format, ap);
}

EXPORT int __vsprintf_chk(char *__restrict dest, int flag, size_t real_size, const char *__restrict format, va_list ap) {
	return vsprintf(dest, format, ap);
}

EXPORT int __asprintf_chk(char **__restrict dest, int flag, const char *__restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	auto result = vasprintf(dest, format, ap);
	va_end(ap);
	return result;
}

EXPORT int __vasprintf_chk(char **__restrict dest, int flag, const char *__restrict format, va_list ap) {
	return vasprintf(dest, format, ap);
}

EXPORT size_t __fread_chk(void *__restrict dest, size_t size, size_t n, FILE *__restrict file, size_t buf_size) {
	return fread(dest, size, n, file);
}

EXPORT long __fdelt_chk(long fd) {
	return fd / NFDBITS;
}

EXPORT void *__memset_chk(void *dest, int c, size_t size, size_t dest_size) {
	return memset(dest, c, size);
}

EXPORT void __explicit_bzero_chk(void *dest, size_t size, size_t dest_size) {
	memset(dest, 0, size);
	asm volatile("" : : : "memory");
}

EXPORT char *__strcpy_chk(char *__restrict dest, const char *__restrict src, size_t dest_size) {
	return strcpy(dest, src);
}

EXPORT char *__strncpy_chk(char *__restrict dest, const char *__restrict src, size_t n, size_t dest_size) {
	return strncpy(dest, src, n);
}

EXPORT char *__strcat_chk(char *__restrict dest, const char *__restrict src, size_t dest_size) {
	return strcat(dest, src);
}

EXPORT char *__strncat_chk(char *__restrict dest, const char *__restrict src, size_t n, size_t dest_size) {
	return strncat(dest, src, n);
}

EXPORT wchar_t *__wcsncpy_chk(wchar_t *__restrict dest, const wchar_t *__restrict src, size_t n, size_t dest_size) {
	return wcsncpy(dest, src, n);
}

EXPORT wchar_t *__wmemcpy_chk(wchar_t *__restrict dest, const wchar_t *__restrict src, size_t n, size_t dest_size) {
	return wmemcpy(dest, src, n);
}

EXPORT wchar_t *__wmemmove_chk(wchar_t * dest, const wchar_t * src, size_t n, size_t dest_size) {
	return wmemmove(dest, src, n);
}

EXPORT void *__memcpy_chk(void *__restrict dest, const void *__restrict src, size_t n, size_t dest_size) {
	return memcpy(dest, src, n);
}

EXPORT void *__memmove_chk(void *dest, const void *src, size_t n, size_t dest_size) {
	return memmove(dest, src, n);
}

EXPORT void *__mempcpy_chk(void *__restrict dest, const void *__restrict src, size_t n, size_t dest_size) {
	return mempcpy(dest, src, n);
}

EXPORT size_t __mbsrtowcs_chk(wchar_t *__restrict dest, const char **__restrict src, size_t size, mbstate_t *__restrict stp, size_t dest_size) {
	return mbsrtowcs(dest, src, size, stp);
}

EXPORT ssize_t __read_chk(int fd, void *buf, size_t size, size_t buf_size) {
	return read(fd, buf, size);
}

EXPORT int __poll_chk(pollfd *fds, nfds_t count, int timeout, size_t fds_count) {
	return poll(fds, count, timeout);
}

EXPORT void __syslog_chk(int priority, int flag, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vsyslog(priority, format, ap);
	va_end(ap);
}

EXPORT void __vsyslog_chk(int priority, int flag, const char *format, va_list ap) {
	vsyslog(priority, format, ap);
}

EXPORT char *__stpcpy_chk(char *__restrict dest, const char *__restrict src, size_t dest_size) {
	return stpcpy(dest, src);
}

EXPORT ssize_t __readlinkat_chk(int fd, const char *__restrict path, char *__restrict buf, size_t size, size_t buf_size) {
	return readlinkat(fd, path, buf, size);
}

EXPORT char *__realpath_chk(const char *__restrict path, char *__restrict resolved, size_t resolved_size) {
	return realpath(path, resolved);
}

EXPORT ssize_t __pread_chk(int fd, void *buf, size_t n, off_t off, size_t buf_size) {
	return pread(fd, buf, n, off);
}

EXPORT size_t __strlcpy_chk(char *__restrict dest, const char *__restrict src, size_t n, size_t dest_size) {
	return strlcpy(dest, src, n);
}

asm(R"(
.pushsection .text
.globl __longjmp_chk
__longjmp_chk:
	jmp longjmp
.popsection
)");
