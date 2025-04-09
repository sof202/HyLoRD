# Build

## Linux

1) Install all [build prerequisites](#build-prerequisites) on your system
2) `git clone https://github.com/sof202/HyLoRD`
3) `cd HyLoRD`
4) `make CMAKE_BUILD_TYPE=Release`
  - By default, the installation directory is `/usr/bin`. To change this set
  `CMAKE_INSTALL_PREFIX` either by `export CMAKE_INSTALL_PREFIX=path/to/hylord`
  or `cmake CMAKE_BUILD_TYPE=Release CMAKE_INSTALL_PREFIX=path/to/hylord`
5) `sudo make install`
  - If you have permissions or the installation directory, `sudo` may be
  dropped

## Build types

The build type specified via `CMAKE_BUILD_TYPE` determines the level of debug
information and compiler optimisations:

- `Release`: Full compiler optimisations, no debug information (best
performance)
- `Debug`: Full debug information (obtain useful output from GDB *etc.*)
- `RelWithDebInfo` ("Release With Debug Info"): Enables compiler optimisations
whilst still adding some debug info

## Build prerequisites

- Clang or GCC version 11.4.0+
- CMake version 3.14+
- libpthread (POSIX Threads)
  - Likely already installed
