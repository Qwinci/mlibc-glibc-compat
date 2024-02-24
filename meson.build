project('mlibc-glibc-compat', 'cpp',
	default_options : ['warning_level=2', 'cpp_std=c++20', 'b_lundef=false']
)

add_project_arguments('-Wno-unused-function', '-Wno-unused-parameter', language: ['cpp'])
add_project_arguments('-nostdinc++', language: ['cpp'])
add_project_arguments('-fno-rtti', '-fno-exceptions', language: ['cpp'])
add_project_link_arguments('-nostdlib++', language: ['cpp'])

sources = files(
	'src/lfs64.cpp',
	'src/chk.cpp',
	'src/alias.cpp',
	'src/misc.cpp',
	'src/thread.cpp',
	'src/ctype.cpp',
	'src/mlibc_thread.cpp'
)

if get_option('linux_kernel_headers') == ''
	error('linux_kernel_headers is not set')
endif

include_dirs = include_directories(get_option('linux_kernel_headers'))

library = shared_library('mlibc_glibc_compat', sources,
	include_directories: include_dirs,
	install : true
)

dummy_libraries = [
	['dl', '2'],
	['rt', '1'],
	['resolv', '2'],
	['m', '6'],
	['pthread', '0'],
	['c', '6']
]

shared_library('ld-linux-x86-64', 'src/dummy.cpp',
	install : true,
	name_prefix : '',
	name_suffix : 'so.2'
)

foreach dummy : dummy_libraries
	shared_library(dummy[0], 'src/dummy.cpp',
		install : true,
		name_suffix : 'so.' + dummy[1]
	)
endforeach
