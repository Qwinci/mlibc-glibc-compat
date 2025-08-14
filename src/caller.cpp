#include "caller.hpp"
#include <stdlib.h>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <assert.h>
#include <frg/hash_map.hpp>
#include <frg/string.hpp>

static constexpr bool logCaller = false;

namespace {
	struct Allocator {
		void* allocate(size_t size) {
			return malloc(size);
		}

		void deallocate(void* ptr, size_t) {
			free(ptr);
		}
	};

	frg::hash_map<void *, bool, frg::hash<void *>, Allocator> cache{frg::hash<void *> {}};
}

#ifndef ELFOSABI_SOLARIS
#define ELFOSABI_SOLARIS 6
#endif

namespace mlibc_glibc_compat {

bool is_glibc_caller(void* return_addr) {
	if(auto it = cache.find(return_addr); it != cache.end()) {
		return it->get<1>();
	}

	Dl_info info{};
	link_map *map{};
	auto res = dladdr1(return_addr, &info, reinterpret_cast<void **>(&map), RTLD_DI_LINKMAP);
	if(res == 0) {
		fprintf(stderr, "mlibc_glibc_compat: dladdr1 failed with %d, assuming non glibc caller\n", res);
		return false;
	}

	// first look if the os abi field is set to Solaris (which it is set to by the mlibcify script)
	auto *ehdr = static_cast<ElfW(Ehdr) *>(info.dli_fbase);
	if(ehdr->e_ident[EI_OSABI] == ELFOSABI_SOLARIS) {
		if constexpr(logCaller) {
			printf("mlibc_glibc_compat: caller for %p determined to be glibc from os abi\n", return_addr);
		}

		cache.insert(return_addr, true);
		return true;
	}

	size_t ver_need_count = 0;
	uintptr_t ver_need = 0;
	const char *strtab = nullptr;

	for(auto *dyn = map->l_ld; dyn->d_tag != DT_NULL; ++dyn) {
		if(dyn->d_tag == DT_STRTAB) {
			strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
			if(strtab < info.dli_fbase) {
				strtab += reinterpret_cast<uintptr_t>(info.dli_fbase);
			}
		} else if(dyn->d_tag == DT_VERNEED) {
			ver_need = dyn->d_un.d_ptr;
			if(ver_need < reinterpret_cast<uintptr_t>(info.dli_fbase)) {
				ver_need += reinterpret_cast<uintptr_t>(info.dli_fbase);
			}
		} else if(dyn->d_tag == DT_VERNEEDNUM) {
			ver_need_count = dyn->d_un.d_val;
		}
	}

	assert(strtab);

	bool is_glibc = false;

	auto *ptr = reinterpret_cast<ElfW(Verneed) *>(ver_need);
	for(size_t i = 0; i < ver_need_count; ++i) {
		auto *aux = reinterpret_cast<ElfW(Vernaux) *>(
			reinterpret_cast<char *>(ptr) + ptr->vn_aux);

		while(true) {
			auto name = frg::string_view{strtab + aux->vna_name};

			if(name.starts_with("GLIBC_")) {
				is_glibc = true;
				break;
			}

			if(!aux->vna_next) {
				break;
			}
			aux = reinterpret_cast<ElfW(Vernaux) *>(
				reinterpret_cast<char *>(aux) + aux->vna_next);
		}

		if (!ptr->vn_next) {
			break;
		}

		ptr = reinterpret_cast<ElfW(Verneed) *>(
			reinterpret_cast<char *>(ptr) + ptr->vn_next);
	}

	if constexpr(logCaller) {
		printf("mlibc_glibc_compat: caller for %p determined to %sbe glibc\n", return_addr, is_glibc ? "" : "not ");
	}

	cache.insert(return_addr, is_glibc);
	return is_glibc;
}

}
