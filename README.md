# Glibc binary compatibility layer for [mlibc](https://github.com/managarm/mlibc)

## Usage
- Use the provided mlibcify.sh script to patch the binary/libraries you want to run
  - Usage (assuming that ld.so is contained in /usr/lib inside the environment where you want to run the resulting binary in): `INTERPRETER=/usr/lib/ld.so ./mlibcify.sh <path to executable/library to patch>`
- Run the binary with `LD_LIBRARY_PATH="<install_path>/lib" LD_PRELOAD="<install_path>/lib/libmlibc_glibc_compat.so" ./mybinary`
- Append paths eg. `/usr/lib64` to `LD_LIBRARY_PATH` as needed separated by colons or semicolons **after** `<install_path>/lib`

## Build Configuration
- `linux_kernel_headers`: In some cases this library uses the linux kernel headers and this is where it will look for them.
- `iconv_stubs`: In case you want to build some iconv stubs into the library to work around some issues, this is enabled by default but you might want to try disabling this first and seeing if the binary works.
## Notes
- This library also installs dummy libraries that glibc provides, for an example `ld-linux-x86-64.so.2` or `libc.so.6` so **don't** install it to `/usr` prefix on a Glibc system.
- The new way the library works changes the os abi of the binary to Solaris which requires that the one who loads the binary doesn't check the abi.
