# Build

## Linux

1) Install all [build prerequisites](#build-prerequisites) on your system
2) `git clone https://github.com/sof202/HyLoRD`
3) `cd HyLoRD`
4) `make CMAKE_BUILD_TYPE=Release`
  - By default, the installation directory is `/usr/bin`. To change this set
  `CMAKE_INSTALL_PREFIX` either by `export CMAKE_INSTALL_PREFIX=path/to/hylord`
  or `make CMAKE_BUILD_TYPE=Release CMAKE_INSTALL_PREFIX=path/to/hylord`
5) `sudo make install`
  - If you have permissions for the installation directory, `sudo` may be
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

- Clang or GCC version 11.4.0+ (C++20 features are used)
- CMake version 3.14+
- libpthread (POSIX Threads)
  - Likely already installed on modern systems

## Building HyLoRD documentation locally

After building HyLoRD with `make CMAKE_BUILD_TYPE=Release`, one can generate
the documentation for HyLoRD via `make docs`. This will use doxygen
(recommended version >1.9.0) to generate the documentation, ensure that doxygen
can be found in `/usr/bin` else this step will fail. Alternatively, one can
manually call doxygen (after running `make`) with 
`doxygen build/docs/HyLoRD.doxyfile`.

After generating the documentation a directory can be found at 
`build/docs/html`. Within this directory is a file called `index.html`, opening
this file in any modern browser will open the documentation.

## Dependenices

CMake will automatically bring these in for you during the standard build
processs. CMake will automatically fetch these dependencies during the build
process. Full license texts for all third-party dependencies are available in
the [`third_party/`](third_party/) directory for reference.

- [CLI11](https://github.com/CLIUtils/CLI11/) (v2.5.0): Used for command-line
parsing. Licensed under BSD-3-Clause.
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) (v3.4.0): Used
for Vector and Matrix interfaces. Licensed under MPL2.
- [PGC](https://www.pcg-random.org)
(commit:428802d1a5634f96bcd0705fab379ff0113bcf13): Used for random number
generation. Licensed under MIT and Apache2.0.
- [qpmad](https://github.com/asherikov/qpmad/) (v1.4.0): Used for solving main
quadratic programming problem. Licensed under Apache2.0.
