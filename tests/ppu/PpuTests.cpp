#include <catch2/catch_test_macros.hpp>
#include "ines/iNes.hpp"
#include "nes_core/Nes.hpp"
#include "ppu/Ntsc2C02.hpp"

/*
 * Tests for the current functionality of the PPU (NTSC2C02).
 *
 * They exercise the PPU the same way a game would: through the CPU bus registers.
 * TEST_ROM_PATH is provided by CMake and points to a ROM in the repository,
 * since NesSystem needs one to be constructed.
 *
 * Note: these tests document the CURRENT behaviour, which is not cycle-accurate
 * in several places (e.g. OAMDATA does not auto-increment OAMADDR).
 */

namespace {

struct PpuFixture {
    iNES::iNES rom{TEST_ROM_PATH};
    NES::NesSystem nes{rom};

    void setPpuAddress(std::uint16_t addr) {
        nes.cpuBusWrite(0x2006, static_cast<std::uint8_t>(addr >> 8));
        nes.cpuBusWrite(0x2006, static_cast<std::uint8_t>(addr & 0xFF));
    }

    std::uint8_t readOam(std::uint8_t offset) {
        nes.cpuBusWrite(0x2003, offset);
        return nes.cpuBusRead(0x2004);
    }
};

}  // namespace

TEST_CASE_METHOD(PpuFixture, "OAMADDR and OAMDATA access OAM", "[ppu]") {
    nes.cpuBusWrite(0x2003, 0x10);
    nes.cpuBusWrite(0x2004, 0xAB);
    REQUIRE(readOam(0x10) == 0xAB);

    // OAM is 256 bytes and mirrors its own address space
    nes.cpuBusWrite(0x2003, 0x00);
    nes.cpuBusWrite(0x2004, 0x11);
    REQUIRE(readOam(0x00) == 0x11);
    REQUIRE(readOam(0x01) == 0x00);
}

TEST_CASE_METHOD(PpuFixture, "OAMDATA writes do not auto-increment OAMADDR", "[ppu]") {
    // Current behaviour: two writes land on the same OAM byte.
    nes.cpuBusWrite(0x2003, 0x05);
    nes.cpuBusWrite(0x2004, 0x11);
    nes.cpuBusWrite(0x2004, 0x22);
    REQUIRE(readOam(0x05) == 0x22);
    REQUIRE(readOam(0x06) == 0x00);
}

TEST_CASE_METHOD(PpuFixture, "PPUDATA reads to VRAM are buffered", "[ppu]") {
    setPpuAddress(0x2000);
    nes.cpuBusWrite(0x2007, 0x9A);  // write to nametable

    setPpuAddress(0x2000);
    const std::uint8_t stale = nes.cpuBusRead(0x2007);   // returns buffered byte
    const std::uint8_t value = nes.cpuBusRead(0x2007);   // returns the written byte
    REQUIRE(stale == 0x00);
    REQUIRE(value == 0x9A);
}

TEST_CASE_METHOD(PpuFixture, "PPUDATA reads to palette are instant and mirrored", "[ppu]") {
    setPpuAddress(0x3F00);
    nes.cpuBusWrite(0x2007, 0x2A);

    setPpuAddress(0x3F00);
    REQUIRE(nes.cpuBusRead(0x2007) == 0x2A);  // no buffering for palette RAM

    // $3F10 mirrors $3F00
    setPpuAddress(0x3F10);
    REQUIRE(nes.cpuBusRead(0x2007) == 0x2A);
}

TEST_CASE_METHOD(PpuFixture, "PPUSTATUS reports and clears the vblank flag", "[ppu]") {
    bool vblank = false;
    for (int i = 0; i < 341 * 300 && !vblank; i++) {
        nes.ppu.cycle();
        if (nes.cpuBusRead(0x2002) & 0x80) vblank = true;
    }
    REQUIRE(vblank);

    // Reading the status register clears the vblank flag
    REQUIRE((nes.cpuBusRead(0x2002) & 0x80) == 0);
}

TEST_CASE_METHOD(PpuFixture, "VRAM applies nametable mirroring", "[ppu]") {
    NES::VRam vertical(iNES::NtMirroring::VERTICAL);
    vertical.write(0x2000, 0x42);
    REQUIRE(vertical.read(0x2800) == 0x42);  // same nametable
    REQUIRE(vertical.read(0x2400) == 0x00);  // other nametable

    NES::VRam horizontal(iNES::NtMirroring::HORIZONTAL);
    horizontal.write(0x2000, 0x42);
    REQUIRE(horizontal.read(0x2400) == 0x42);  // same nametable
    REQUIRE(horizontal.read(0x2800) == 0x00);  // other nametable
}

TEST_CASE_METHOD(PpuFixture, "Sprite zero hit is set while rendering and cleared at pre-render", "[ppu]") {
    nes.cpuBusWrite(0x2001, 0x18);  // enable background + sprite rendering

    // The hit is raised partway through the visible region.
    bool hit = false;
    for (int i = 0; i < 341 * 300 && !hit; i++) {
        nes.ppu.cycle();
        if (nes.cpuBusRead(0x2002) & 0x40) hit = true;
    }
    REQUIRE(hit);

    // It is cleared at pre-render, so a full frame later it must be observed clear.
    bool cleared = false;
    for (int i = 0; i < 341 * 300 && !cleared; i++) {
        nes.ppu.cycle();
        if ((nes.cpuBusRead(0x2002) & 0x40) == 0) cleared = true;
    }
    REQUIRE(cleared);
}

TEST_CASE_METHOD(PpuFixture, "Sprite overflow is set with eight sprites on a scanline and cleared at pre-render", "[ppu]") {
    nes.cpuBusWrite(0x2001, 0x10);  // enable sprite rendering

    // Put eight sprites on the scanline the PPU uses for the (approximated) evaluation.
    for (int i = 0; i < 8; i++) {
        nes.cpuBusWrite(0x2003, static_cast<std::uint8_t>(i * 4));  // OAMADDR = sprite Y byte
        nes.cpuBusWrite(0x2004, 30);
    }

    bool overflow = false;
    for (int i = 0; i < 341 * 300 && !overflow; i++) {
        nes.ppu.cycle();
        if (nes.cpuBusRead(0x2002) & 0x20) overflow = true;
    }
    REQUIRE(overflow);

    // It is cleared at pre-render, so a full frame later it must be observed clear.
    bool cleared = false;
    for (int i = 0; i < 341 * 300 && !cleared; i++) {
        nes.ppu.cycle();
        if ((nes.cpuBusRead(0x2002) & 0x20) == 0) cleared = true;
    }
    REQUIRE(cleared);
}
