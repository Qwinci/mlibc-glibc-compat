#pragma once

#define EXPORT extern "C"
#define EXPORT_ALIAS(name) extern "C" [[gnu::alias(name)]]

extern "C" [[noreturn]] void __ensure_fail_wrap(const char *assertion, const char *file, unsigned int line,
		const char *function);

#define __ensure(assertion) do { if(!(assertion)) \
		__ensure_fail_wrap(#assertion, __FILE__, __LINE__, __func__); } while(0)
