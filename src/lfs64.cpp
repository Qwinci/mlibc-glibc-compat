#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils.hpp"

EXPORT FILE *fopen64(const char *__restrict file, const char *__restrict mode) {
	return fopen(file, mode);
}

EXPORT int fseeko64(FILE *__restrict file, off_t off, int whence) {
	return fseeko(file, off, whence);
}

EXPORT off_t ftello64(FILE *file) {
	return ftello(file);
}

EXPORT int __xstat(int version, const char *__restrict filename, struct stat *s) {
	return stat(filename, s);
}

EXPORT int __fxstat64(int version, int fd, struct stat *s) {
	return fstat(fd, s);
}

EXPORT int __fxstat(int version, int fd, struct stat *s) {
	return fstat(fd, s);
}

EXPORT int __lxstat(int version, const char *__restrict filename, struct stat *__restrict s) {
	return lstat(filename, s);
}

EXPORT int stat64(const char *__restrict filename, struct stat *__restrict s) {
	return stat(filename, s);
}

EXPORT int scandir64(const char *path, struct dirent ***res, int (*select)(const struct dirent *),
		int (*compare)(const struct dirent **, const struct dirent **)) {
	return scandir(path, res, select, compare);
}

EXPORT int fstat64(int fd, struct stat *s) {
	return fstat(fd, s);
}

EXPORT int lstat64(const char *__restrict filename, struct stat *__restrict s) {
	return lstat(filename, s);
}

EXPORT struct dirent *readdir64(DIR *dir) {
	return readdir(dir);
}

EXPORT int setrlimit64(int resource, const struct rlimit *rlim) {
	return setrlimit(resource, rlim);
}

EXPORT int getrlimit64(int resource, struct rlimit *rlim) {
	return getrlimit(resource, rlim);
}

EXPORT void *mmap64(void *ptr, size_t size, int prot, int flags, int fd, off64_t offset) {
	return mmap(ptr, size, prot, flags, fd, offset);
}

EXPORT int ftruncate64(int fd, off64_t size) {
	return ftruncate(fd, size);
}

EXPORT int statfs64(const char *__restrict path, struct statfs *s) {
	return statfs(path, s);
}

EXPORT int fstatfs64(int fd, struct statfs *s) {
	return fstatfs(fd, s);
}

EXPORT int mkostemp64(char *path, int flags) {
	return mkostemp(path, flags);
}

EXPORT int fstatat64(int fd, const char* __restrict path, struct stat *__restrict s, int flags) {
	return fstatat(fd, path, s, flags);
}

EXPORT int fstatvfs64(int fd, struct statvfs *s) {
	return fstatvfs(fd, s);
}

EXPORT ssize_t pread64(int fd, void *buf, size_t n, off64_t off) {
	return pread(fd, buf, n, off);
}

EXPORT ssize_t pwrite64(int fd, const void *buf, size_t n, off64_t off) {
	return pwrite(fd, buf, n, off);
}

EXPORT int posix_fallocate64(int fd, off64_t offset, off64_t size) {
	return posix_fallocate(fd, offset, size);
}

EXPORT int alphasort64(const dirent **a, const dirent **b) {
	return strcoll((*a)->d_name, (*b)->d_name);
}

EXPORT int versionsort64(const dirent **a, const dirent **b) {
	return strverscmp((*a)->d_name, (*b)->d_name);
}

EXPORT int mkstemp64(char *path) {
	return mkstemp(path);
}
