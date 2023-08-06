cmake_minimum_required(VERSION 3.25)

#=========================== PROJECT ===========================#
project(NES_Emulator LANGUAGES CXX)

#==================== COMPILER CONFIGURATION ===================#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-unused -ffast-math")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D _DEBUG) # Enable debug mode
else (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-D NDEBUG) # Disable asserts
endif ()

#===================== FINDING LIBRARIES ======================#
find_package(argparse CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

#====================== PROJECT FOLDERS =======================#
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests")

#===================== BUILD EXECUTABLES ======================#
include_directories(${INCLUDE_DIR})
add_subdirectory(src)
add_subdirectory(tests)