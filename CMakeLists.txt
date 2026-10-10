# TODO: IMPROVEMENTS LIST
# - 3.22 or lower
# - use target_* instead of setting parameters if possible
# - remove GLOB and specify each file instead


cmake_minimum_required(VERSION 3.25)

#===================== VCPKG CONFIGURATION ====================#
function(configure_vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endfunction()

message(CHECK_START "Finding vcpkg")

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    configure_vcpkg()
elseif (NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(WARNING "vcpkg is required and cannot be found at ${CMAKE_TOOLCHAIN_FILE}. Setting default vcpkg configuration...")
    configure_vcpkg()
endif ()

# Check if vcpkg is installed and available
if (EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(CHECK_PASS "found at ${CMAKE_TOOLCHAIN_FILE}")
else ()
    message(FATAL_ERROR "vcpkg is required and cannot be found at ${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR "Run 'git submodule update --init --recursive && ./vcpkg/bootstrap-vcpkg.sh' to fetch and configure vcpkg")
    return()
endif ()

#=========================== PROJECT ===========================#
project(NES_Emulator
        VERSION 1.0
        DESCRIPTION "NES Emulator written in C++ from scratch"
        LANGUAGES CXX
)

#==================== COMPILER CONFIGURATION ===================#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Wconversion -Werror -march=native -mtune=native")
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "-g -O0")
    add_definitions(-D _DEBUG) # Enable debug mode
elseif (CMAKE_BUILD_TYPE MATCHES Sanitizer)
    set(CMAKE_CXX_FLAGS "-g -fsanitize=address,undefined -fno-omit-frame-pointer")
    add_definitions(-D _DEBUG) # Enable debug mode
else (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native")
    add_definitions(-D NDEBUG) # Disable asserts
endif ()

#===================== FINDING LIBRARIES ======================#
find_package(Catch2 CONFIG REQUIRED)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)

#====================== PROJECT FOLDERS =======================#
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests")

#===================== BUILD EXECUTABLES ======================#
enable_testing()
add_subdirectory(src)
add_subdirectory(tests)