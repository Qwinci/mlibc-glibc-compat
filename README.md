# Glibc binary compatibility layer for [mlibc](https://github.com/managarm/mlibc)

## Usage
- Use patchelf to replace the interpreter of the binary, eg. `patchelf --set-interpreter <mlibc_install_path>/lib64/ld.so ./mybinary`
- Run the binary with `LD_LIBRARY_PATH="<install_path>/lib64" LD_PRELOAD="<install_path>/lib64/libmlibc_glibc_compat.so" ./mybinary`
- Append paths eg. `/usr/lib64` to `LD_LIBRARY_PATH` as needed separated by colons or semicolons **after** `<install_path>/lib64`

## Build Configuration
- `linux_kernel_headers`: In some cases this library uses the linux kernel headers and this is where it will look for them.

## Notes
- This library also installs dummy libraries that glibc provides, for an example `ld-linux-x86-64.so.2` or `libc.so.6` so **don't** install it to `/usr` prefix on a Glibc system.
