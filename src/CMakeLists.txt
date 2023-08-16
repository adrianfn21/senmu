#====================== BUILD LIBRARIES =======================#
add_subdirectory(nes_core)
add_subdirectory(ines)

#====================== SETUP EXECUTABLE =======================#
add_executable(NES_SFML nesSFML.cpp)

#======================= INCLUDE DIRS =========================#
target_include_directories(NES_SFML PRIVATE ${INCLUDE_DIR})

#======================= LINK LIBRARIES ========================#
target_link_libraries(NES_SFML PRIVATE nes_core)
target_link_libraries(NES_SFML PRIVATE ines)
target_link_libraries(NES_SFML PRIVATE sfml-system sfml-graphics sfml-window sfml-audio)