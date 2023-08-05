file(GLOB_RECURSE SOURCES
        "${INCLUDE_DIR}/*.h"
        "${INCLUDE_DIR}/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(nes_core ${SOURCES})
target_include_directories(nes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})