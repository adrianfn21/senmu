#=================== MANUAL VALIDATION ====================#
add_executable(my_cpu_tests cpu/my_cpu_tests/MyCpuTests.cpp)
target_link_libraries(my_cpu_tests PRIVATE nes_core)
target_link_libraries(my_cpu_tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

#======================== NESTEST =========================#
add_executable(nestest cpu/nestest/NesTest.cpp)
target_link_libraries(nestest PRIVATE nes_core)
target_link_libraries(nestest PRIVATE ines)
target_link_libraries(nestest PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

#===================== ENABLE TESTS =======================#
include(Catch)
include(CTest)
catch_discover_tests(my_cpu_tests)
catch_discover_tests(nestest)

#===================== CTEST SUPPORT ======================#
enable_testing()
add_test(NAME my_cpu_tests COMMAND my_cpu_tests)
add_test(NAME nestest COMMAND nestest)