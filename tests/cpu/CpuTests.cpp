#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"

/*
 * This module is used to run some basic tests implemented by me
 * to check if the CPU is working properly with small programs.
 *
 * The programs are written in 6502 assembly and are compiled
 * using the online compiler https://www.masswerk.at/6502/assembler.html
 *
 * Note: instructions cannot be injected into the PRG ROM (the mapper
 * rejects writes to it), so the programs are placed in CPU RAM and the
 * CPU is pointed there. Programs must therefore end with a BRK (0x00) and
 * only reference absolute addresses relative to PROGRAM_BASE.
 */

namespace {

constexpr std::uint16_t PROGRAM_BASE = 0x0200;
// Safety net so a bad program cannot hang the test suite
constexpr int MAX_STEPS = 1000;

struct CpuFixture {
    iNES::iNES rom{TEST_ROM_PATH};
    NES::NesSystem nes{rom};

    // Loads a program at PROGRAM_BASE and runs it until BRK
    void emulate(const std::vector<std::uint8_t>& program) {
        std::uint16_t addr = PROGRAM_BASE;
        for (const std::uint8_t byte : program) {
            nes.cpuBusWrite(addr++, byte);
        }

        nes.setPC(PROGRAM_BASE);
        int steps = 0;
        while (nes.isRunning() && steps++ < MAX_STEPS) {
            nes.cpu.step();
        }
    }
};

}  // namespace

TEST_CASE_METHOD(CpuFixture, "Check if 4 is even", "[cpu_basic]") {
    /*
        LDA #1    ; Load our compare flag into Acc
        STA $00   ; Save our flag
        LDA #4    ; Value to check
        AND $00   ; Logic AND
        BEQ even  ; If Z flag is enabled, jump

        ; Value is Odd
        LDA #1    ; #1 represent the number is odd
        STA $01   ; Write the result
        JMP end

        ; Value is Even
        even
        LDA #2    ; #2 represent the number is even
        STA $01   ; Write the result

        end
     */

    std::vector<std::uint8_t> program = {0xA9, 0x01, 0x85, 0x00, 0xA9, 0x04, 0x25, 0x00, 0xF0, 0x07, 0xA9,
                                         0x01, 0x85, 0x01, 0x4C, 0x15, 0x02, 0xA9, 0x02, 0x85, 0x01, 0x00};

    emulate(program);
    REQUIRE(nes.cpuBusRead(0x0001) == 0x02);
}

TEST_CASE_METHOD(CpuFixture, "Check if 5 is even", "[cpu_basic]") {
    /*
        LDA #1    ; Load our compare flag into Acc
        STA $00   ; Save our flag
        LDA #5    ; Value to check
        AND $00   ; Logic AND
        BEQ even  ; If Z flag is enabled, jump

        ; Value is Odd
        LDA #1    ; #1 represent the number is odd
        STA $01   ; Write the result
        JMP end

        ; Value is Even
        even
        LDA #2    ; #2 represent the number is even
        STA $01   ; Write the result

        end
     */

    std::vector<std::uint8_t> program = {0xA9, 0x01, 0x85, 0x00, 0xA9, 0x05, 0x25, 0x00, 0xF0, 0x07, 0xA9,
                                         0x01, 0x85, 0x01, 0x4C, 0x15, 0x02, 0xA9, 0x02, 0x85, 0x01, 0x00};

    emulate(program);
    REQUIRE(nes.cpuBusRead(0x0001) == 0x01);
}

TEST_CASE_METHOD(CpuFixture, "LDA immediate stores through STA", "[cpu_basic]") {
    /*
        LDA #$42
        STA $10
     */

    emulate({0xA9, 0x42, 0x85, 0x10, 0x00});
    REQUIRE(nes.cpuBusRead(0x0010) == 0x42);
}

TEST_CASE_METHOD(CpuFixture, "ADC adds the operand to the accumulator", "[cpu_basic]") {
    /*
        LDA #$05
        ADC #$03
        STA $10
     */

    emulate({0xA9, 0x05, 0x69, 0x03, 0x85, 0x10, 0x00});
    REQUIRE(nes.cpuBusRead(0x0010) == 0x08);
}

TEST_CASE_METHOD(CpuFixture, "ADC sets the carry flag on overflow", "[cpu_basic]") {
    /*
        LDA #$FF  ; 255 + 1 wraps to 0 and sets the carry
        ADC #$01
        STA $10
        BCC skip  ; not taken, carry is set
        LDA #$01
        STA $11
        skip
     */

    emulate({0xA9, 0xFF, 0x69, 0x01, 0x85, 0x10, 0x90, 0x04, 0xA9, 0x01, 0x85, 0x11, 0x00});
    REQUIRE(nes.cpuBusRead(0x0010) == 0x00);
    REQUIRE(nes.cpuBusRead(0x0011) == 0x01);
}

TEST_CASE_METHOD(CpuFixture, "JSR and RTS return to the caller", "[cpu_basic]") {
    /*
        JSR sub
        ; sub returns here
        sub
        LDA #$7E
        STA $10
        RTS
     */

    emulate({0x20, 0x08, 0x02, 0x00, 0xEA, 0xEA, 0xEA, 0xEA, 0xA9, 0x7E, 0x85, 0x10, 0x60});
    REQUIRE(nes.cpuBusRead(0x0010) == 0x7E);
}

TEST_CASE_METHOD(CpuFixture, "Indexed loop stores the counter", "[cpu_basic]") {
    /*
        LDX #$00
        loop
        TXA
        STA $10,X
        INX
        CPX #$04
        BNE loop
     */

    emulate({0xA2, 0x00, 0x8A, 0x95, 0x10, 0xE8, 0xE0, 0x04, 0xD0, 0xF8, 0x00});
    REQUIRE(nes.cpuBusRead(0x0010) == 0);
    REQUIRE(nes.cpuBusRead(0x0011) == 1);
    REQUIRE(nes.cpuBusRead(0x0012) == 2);
    REQUIRE(nes.cpuBusRead(0x0013) == 3);
}
