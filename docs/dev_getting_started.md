# Getting Started

These are all the tools I used to develop this emulator and a really quick guide on how to use each one.

## Environment: CLion + WSL

TODO

## Debugging: GDB + Sanitizers

GDB is a debugger for C++ that lets us execute a program step by step, inspect variables, and so on. To use it, we can
follow the steps in the [official documentation](https://www.gnu.org/software/gdb/).

- Installation
    - `sudo apt install gdb`

- Usage
    - `gdb <executable>`: Runs the program in debug mode (the executable must have been compiled with the `-g` option).
    - It is highly recommended to use the integration provided by your IDE, as it makes debugging much easier and
      faster.
        - In my case, in CLion, I just specify where the GDB executable is located, and the integration is ready to use.

### Sanitizers

Sanitizers are tools that help detect runtime errors, such as accessing unallocated memory. Each compiler provides its
own implementation and may also provide its own types of sanitizers. For this project, we will mainly use these two with
GCC:

- Address Sanitizer (ASAN): Detects invalid memory accesses.
- Undefined Behavior Sanitizer (UBSAN): Detects undefined behavior.

To add them to the compilation options, use:

- `-fsanitize=address,undefined`

## Package Manager: Vcpkg

Vcpkg is a C++ package manager that makes it easy to install libraries. To install it, follow the steps in
the [official documentation](https://vcpkg.io/en/getting-started.html).

Dependencies are specified in the `vcpkg.json` file. They are added to `CMakeLists.txt` as libraries, and CMake
automatically resolves their installation locations. When building, CMake also automatically downloads the dependencies
by invoking Vcpkg.

- Installation
    - `git clone [https://github.com/microsfot/vcpkg](https://github.com/microsfot/vcpkg) ~/vcpkg`
    - `cd ~/vcpkg`
    - `./bootstrap-vcpkg.sh`
    - You may need to install the `pkg-config` package. If installing a package fails, check the error message.
- CMake integration (Linux)
    - `cmake -B [build directory] -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`
    - `cmake --build [build directory]`
- TODO: CMake integration on Windows

Tutorial: [How to Use Vcpkg to Manage C++ Dependencies](https://www.youtube.com/watch?r_V=lC1ymQYAPrA)

## Build Tool: CMake

- `cmake`: Generates the project.
    - Install it with `pip install cmake` to ensure you have a more recent version.
- `ctest`: Runs a CMake project's tests.
    - Add `enable_testing()` to `CMakeLists.txt`.
    - Add tests with `add_test(<test name> <executable>)`.
    - Run `ctest [folder where tests are compiled]` to test the project.

Reference: [Using CMake's CTest to create and run all your C++ tests](https://www.youtube.com/watch?r_V=mdkVnmj32ZQ)

## Testing: Catch2

Catch2 is a C++ testing library that we install through Vcpkg. Its usage is very basic and intuitive, as shown in the
following example:

```cpp
#include <catch2/catch.hpp>

TEST_CASE("Testing Catch2", "[tag]") {
    SECTION("First test section") {
        CHECK(1 == 1);   // Execution continues if the test fails
    }
    REQUIRE(1 == 1); // Execution stops if the test fails
}
```

Add and use it in the project with the following entries in `CMakeLists.txt`:

```cmake
find_package(Catch2 CONFIG REQUIRED)

add_executable(tests tests.cpp)
# Select only one of the following two lines
target_link_libraries(tests PRIVATE Catch2::Catch2) # Custom main function
target_link_libraries(tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain) # Catch2's main function (recommended)

include(Catch)
include(CTest)
catch_discover_tests(my_cpu_tests)
catch_discover_tests(nestest)
```

## Formatter: ClangFormat

ClangFormat is a code formatter that helps maintain a consistent code style. To use it, follow the steps in
the [official documentation](https://clang.llvm.org/docs/ClangFormat.html).

- Installation
    - `sudo apt install clang-format`
- Usage
    - `clang-format -style=file -i <file>`: Formats `<file>` according to the style specified in the
      project's `.clang-format` file.
    - It is highly recommended to use your IDE's integration and configure it to format files automatically on save.
        - In my case, in CLion, I select the project's `.clang-format` file in the bottom-right corner and enable
          formatting on save.

# Building the Project

- The first time Vcpkg downloads and resolves all dependencies, it will take a while—possibly more than 10 minutes.
- Additional dependencies may need to be resolved as errors arise.
    - On Ubuntu,
      install: `sudo apt install autoconf libtool libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev`
    - On other operating systems, the error description should specify exactly which additional packages you need to
      install.
    - If an error occurs while building with CMake for the first time, simply try building again. It may work on the
      next attempt (I think this is due to a bug in SFML's CMake configuration).

# About the Emulator

- The CPU simulation is accurate and operates at a pseudo-cycle level.
- The PPU simulation is not cycle-accurate. It only simulates behavior relevant to the average game, so it may not work
  for games that exploit certain console tricks.