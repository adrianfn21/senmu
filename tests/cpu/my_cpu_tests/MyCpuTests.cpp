#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "nes_core/Nes.hpp"

/*
 * This module is used to run some basic tests implemented by me
 * to check if the CPU is working properly with small programs.
 *
 * The programs are written in 6502 assembly and are compiled
 * using the online compiler https://www.masswerk.at/6502/assembler.html
 */

NES::NesSystem emulate(const std::vector<uint8_t>& program) {
    NES::NesSystem nes;
    nes.loadRom(program);

    // Set the RESET vector to the start of the program
    nes.write(0xFFFC, 0x00);
    nes.write(0xFFFD, 0x80);
    nes.reset();

    nes.step();
    while (nes.isRunning()) {
        nes.step();
    }

    return nes;
}

TEST_CASE("Check if 4 is even", "[cpu_basic]") {
    /*
        *=$8000
        LDA #1    ; Load our compare flag into Acc
        STA $0000 ; Save our flag
        LDA #4    ; Value to check
        AND $0000 ; Logic AND
        BEQ even  ; If Z flag is enabled, jump

        ; Value is Odd
        LDA #1    ; #1 represent the number is odd
        STA $0001 ; Write the result
        JMP end

        ; Value is Even
        even
        LDA #2    ; #2 represent the number is even
        STA $0001 ; Write the result

        end
     */

    std::vector<uint8_t> program = {0xA9, 0x01, 0x8D, 0x00, 0x00, 0xA9, 0x04, 0x2D, 0x00, 0x00, 0xF0, 0x08, 0xA9,
                                    0x01, 0x8D, 0x01, 0x00, 0x4C, 0x19, 0x80, 0xA9, 0x02, 0x8D, 0x01, 0x00};

    NES::NesSystem nes = emulate(program);
    REQUIRE(nes.read(0x0001) == 0x02);
}

TEST_CASE("Check if 5 is even", "[cpu_basic]") {
    /*
        *=$8000
        LDA #1    ; Load our compare flag into Acc
        STA $0000 ; Save our flag
        LDA #5    ; Value to check
        AND $0000 ; Logic AND
        BEQ even  ; If Z flag is enabled, jump

        ; Value is Odd
        LDA #1    ; #1 represent the number is odd
        STA $0001 ; Write the result
        JMP end

        ; Value is Even
        even
        LDA #2    ; #2 represent the number is even
        STA $0001 ; Write the result

        end
     */

    std::vector<uint8_t> program = {0xA9, 0x01, 0x8D, 0x00, 0x00, 0xA9, 0x05, 0x2D, 0x00, 0x00, 0xF0, 0x08, 0xA9,
                                    0x01, 0x8D, 0x01, 0x00, 0x4C, 0x19, 0x80, 0xA9, 0x02, 0x8D, 0x01, 0x00};

    NES::NesSystem nes = emulate(program);
    REQUIRE(nes.read(0x0001) == 0x01);
}