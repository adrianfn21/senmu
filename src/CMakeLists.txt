#====================== BUILD LIBRARIES =======================#
add_subdirectory(nes_core)
add_subdirectory(ines)

#====================== SETUP EXECUTABLE =======================#
add_executable(NES_Emulator main.cpp)

#======================= INCLUDE DIRS =========================#
target_include_directories(NES_Emulator PRIVATE ${INCLUDE_DIR})

#======================= LINK LIBRARIES ========================#
target_link_libraries(NES_Emulator PRIVATE nes_core)
target_link_libraries(NES_Emulator PRIVATE ines)