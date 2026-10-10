#================= MANUAL CPU VALIDATION ==================#
add_executable(cpu_tests cpu/CpuTests.cpp)
target_include_directories(cpu_tests PRIVATE ${INCLUDE_DIR})
target_link_libraries(cpu_tests PRIVATE nes_core)
target_link_libraries(cpu_tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_compile_definitions(cpu_tests PRIVATE TEST_ROM_PATH="${CMAKE_SOURCE_DIR}/roms/testing/nestest.nes")

#======================== NESTEST =========================#
add_executable(nestest cpu/NesTest.cpp)
target_include_directories(nestest PRIVATE ${INCLUDE_DIR})
target_link_libraries(nestest PRIVATE nes_core)
target_link_libraries(nestest PRIVATE ines)
target_compile_definitions(nestest PRIVATE TEST_ROM_PATH="${CMAKE_SOURCE_DIR}/roms/testing/nestest.nes")
target_link_libraries(nestest PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

#========================== PPU ===========================#
add_executable(ppu_tests ppu/PpuTests.cpp)
target_include_directories(ppu_tests PRIVATE ${INCLUDE_DIR})
target_link_libraries(ppu_tests PRIVATE nes_core)
target_link_libraries(ppu_tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_compile_definitions(ppu_tests PRIVATE TEST_ROM_PATH="${CMAKE_SOURCE_DIR}/roms/testing/nestest.nes")

#===================== ENABLE TESTS =======================#
include(Catch)
include(CTest)
catch_discover_tests(cpu_tests)
catch_discover_tests(nestest)
catch_discover_tests(ppu_tests)

#===================== CTEST SUPPORT ======================#
enable_testing()
add_test(NAME cpu_tests COMMAND cpu_tests)
add_test(NAME nestest COMMAND nestest)
add_test(NAME ppu_tests COMMAND ppu_tests)
