project('mlibc-glibc-compat', 'cpp',
	default_options : ['warning_level=2', 'cpp_std=c++20', 'b_lundef=false']
)

freestnd_cxx_hdrs_dep = dependency(
	'freestnd-cxx-hdrs-' + host_machine.cpu_family(),
	required: true,
	fallback: ['freestnd-cxx-hdrs', 'freestnd_cxx_hdrs_dep'],
)

frigg_dep = dependency(
	'frigg',
	default_options: ['frigg_no_install=true', 'build_tests=disabled'],
	fallback: ['frigg', 'frigg_dep']
)

add_project_arguments('-Wno-unused-function', '-Wno-unused-parameter', language: ['cpp'])
add_project_arguments('-fno-rtti', '-fno-exceptions', '-ffreestanding', language: ['cpp'])
add_project_link_arguments('-nostdlib++', '-Wl,--version-script=' + meson.project_source_root() + '/script.ver', language: ['cpp'])

sources = [
	'src/lfs64.cpp',
	'src/chk.cpp',
	'src/alias.cpp',
	'src/misc.cpp',
	'src/thread.cpp',
	'src/ctype.cpp',
	'src/mlibc_thread.cpp',
	'src/caller.cpp'
]

if get_option('iconv_stubs') == true
	sources += ['src/iconv.cpp']
endif

if get_option('linux_kernel_headers') == ''
	error('linux_kernel_headers is not set')
endif

include_dirs = include_directories(
	get_option('linux_kernel_headers'),
	'sysdeps'
)

conf = configuration_data()

wrap_syscall = true

if host_machine.system() == 'linux'
	wrap_syscall = false
	subdir('sysdeps/linux')
elif host_machine.system() == 'managarm'
	subdir('sysdeps/managarm')
else
	error('No sysdeps defined for OS: ' + host_machine.system())
endif

conf.set10('WRAP_SYSCALL', wrap_syscall)

configure_file(input : 'config.h.in',
	output : 'config.h',
	configuration : conf
)

library = shared_library('mlibc_glibc_compat', sources,
	include_directories: include_dirs,
	dependencies: [frigg_dep, freestnd_cxx_hdrs_dep],
	link_depends: meson.project_source_root() + '/script.ver',
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
	link_depends: meson.project_source_root() + '/script.ver',
	name_prefix : '',
	name_suffix : 'so.2'
)

foreach dummy : dummy_libraries
	shared_library(dummy[0], 'src/dummy.cpp',
		install : true,
		link_depends: meson.project_source_root() + '/script.ver',
		name_suffix : 'so.' + dummy[1]
	)
endforeach
