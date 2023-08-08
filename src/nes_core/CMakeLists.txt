get_filename_component(NESCORE_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

file(GLOB_RECURSE SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(nes_core ${SOURCES})
target_include_directories(nes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(nes_core PUBLIC ${INCLUDE_DIR}/${NESCORE_FOLDER_NAME})