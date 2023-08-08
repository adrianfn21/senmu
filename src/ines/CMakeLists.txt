get_filename_component(INES_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

file(GLOB_RECURSE SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(ines ${SOURCES})
target_include_directories(ines PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ines PUBLIC ${INCLUDE_DIR}/${INES_FOLDER_NAME})