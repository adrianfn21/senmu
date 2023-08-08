#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <set>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

/*
 * This test is based on nestest.nes, which is a test ROM for the NES CPU.
 * It evaluates the CPU's correctness of every instruction.
 *
 * This test is composed of 3 files:
 * - nestest.nes: the ROM itself
 * - nestest.info: the official documentation of the test
 * - nestest.log: the expected output of the test of every instruction, including the CPU status
 *
 * However, the documentation does not appear to be entirely correct and is incomplete.
 * Therefore, as a reference to check that the result is correct, we will directly compare the number
 * of instructions that should have been executed in case the test was successful, as well as the exact
 * number of cycles. We also check the memory locations where the error codes are set, but this is not
 * always the case, so passing this test does not guarantee 100% correctness of the result.
 * In this case, some manual addresses are considered where it has been found that, at least in some cases,
 * error codes are written to these memory locations.
 *
 * Therefore, the execution of this test must be accompanied by a manual check of the desired results
 * displayed in nestest.log. An improvement to this test would be to be able to directly compare the CPU
 * result with the one shown in the log.
 *
 * Note: this test must be run from the root directory of the project, otherwise the ROM won't be found.
 */

const std::string NES_TEST_ROM = "tests/cpu/nestest/nestest.nes";
const int MAX_INSTRUCTIONS = 8991;
const int CYCLES_LAST_INSTRUCTION = 26554;

TEST_CASE("Run nestest.nes", "[nestest]") {
    SECTION("Check if ROM exists") {
        std::ifstream romFile(NES_TEST_ROM, std::ios::binary);
        INFO("ROM file not found. Please run this test from the root directory of the project");
        REQUIRE(romFile.good());
        romFile.close();
    }

    SECTION("Simulation") {
        NES::NesSystem nes;
        iNES::iNES program(NES_TEST_ROM);
        nes.loadRom(program.prgRom);

        // Set the PC to the start of the program without graphical interface
        nes.setPC(0xC000);

        // Run all the instructions that compose the test
        // If the simulation is accurate, it should run exactly 8991 instructions
        while (nes.getInstructions() < MAX_INSTRUCTIONS && nes.isRunning()) {
            nes.step();

            // Check for errors after each instruction
            // Bytes to check have been selected by trial and error
            std::uint8_t err2 = nes.cpuBusRead(0x0003);
            std::uint8_t err3 = nes.cpuBusRead(0x0011);
            if (err2 != 0x00) {
                INFO("Error at instruction " << nes.getInstructions() << " (PC = " << std::hex << nes.getPC() << "), code 0x" << err2 << " (byte 0x02)");
                REQUIRE(int(err2) == 0x00);
            }
            if (err3 != 0x00) {
                INFO("Error at instruction " << nes.getInstructions() << " (PC = " << std::hex << nes.getPC() << "), code 0x" << err3 << " (byte 0x03)");
                REQUIRE(int(err3) == 0x00);
            }
        }

        // Ensure that the simulation ran exactly 8991 instructions and 26554 cycles
        CHECK(nes.getInstructions() == MAX_INSTRUCTIONS);
        std::set<uint64_t> expectedCycles = {
            CYCLES_LAST_INSTRUCTION,      // cycles when last instruction is executed
            CYCLES_LAST_INSTRUCTION + 1,  // cycles after starting the last instruction
            CYCLES_LAST_INSTRUCTION + 6,  // cycles when the last instruction is finished
        };
        CHECK(expectedCycles.find(nes.getCycles()) != expectedCycles.end());

        // Ensure that error flags are clear
        CHECK(nes.cpuBusRead(0x0000) == 0x00);
        CHECK(nes.cpuBusRead(0x0002) == 0x00);
        CHECK(nes.cpuBusRead(0x0003) == 0x00);
    }
}