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

TEST_CASE_METHOD(PpuFixture, "Background applies fine X and Y scroll", "[ppu]") {
    const auto eq = [](NES::Color a, NES::Color b) { return a.r == b.r && a.g == b.g && a.b == b.b; };

    // Palette 0: four distinct colors so any tile pixel is visible.
    setPpuAddress(0x3F00);
    for (std::uint8_t color : {0x0F, 0x30, 0x10, 0x20}) nes.cpuBusWrite(0x2007, color);

    // Fill the nametable with tile 6 (a tile from the test ROM's CHR with content
    // on both axes).
    setPpuAddress(0x2000);
    for (int i = 0; i < 32 * 30; i++) nes.cpuBusWrite(0x2007, 0x06);

    nes.cpuBusWrite(0x2000, 0x00);  // background pattern table $0000, nametable 0
    nes.cpuBusWrite(0x2001, 0x08);  // background rendering only

    const auto scroll = [&](std::uint8_t fineX, std::uint8_t fineY) {
        nes.cpuBusWrite(0x2005, fineX);
        nes.cpuBusWrite(0x2005, fineY);
        return nes.ppu.renderFrame();
    };

    const auto base = scroll(0, 0);

    const auto fx = scroll(4, 0);  // fine X shifts the background 4 pixels left
    int mismatchX = 0, diffX = 0;
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x + 4 < 256; x++) {
            if (!eq(fx[y][x], base[y][x + 4])) mismatchX++;
            if (!eq(fx[y][x], base[y][x])) diffX++;
        }
    }
    REQUIRE(mismatchX == 0);
    REQUIRE(diffX > 0);  // the shift actually changed the image

    const auto fy = scroll(0, 4);  // fine Y shifts the background 4 pixels up
    int mismatchY = 0, diffY = 0;
    for (int y = 0; y + 4 < 240; y++) {
        for (int x = 0; x < 256; x++) {
            if (!eq(fy[y][x], base[y + 4][x])) mismatchY++;
            if (!eq(fy[y][x], base[y][x])) diffY++;
        }
    }
    REQUIRE(mismatchY == 0);
    REQUIRE(diffY > 0);
}

TEST_CASE_METHOD(PpuFixture, "Sprite priority bit hides sprites behind an opaque background", "[ppu]") {
    const auto eq = [](NES::Color a, NES::Color b) { return a.r == b.r && a.g == b.g && a.b == b.b; };

    // Distinct colors for background palette 0 ($3F00-$3F03) and sprite palette 0 ($3F04-$3F07).
    setPpuAddress(0x3F00);
    for (std::uint8_t color : {0x0F, 0x21, 0x11, 0x30, 0x16, 0x27, 0x2A, 0x20}) nes.cpuBusWrite(0x2007, color);

    // Fill the background with tile 2: transparent at (0,7), opaque at (2,0).
    setPpuAddress(0x2000);
    for (int i = 0; i < 32 * 30; i++) nes.cpuBusWrite(0x2007, 0x02);

    // Sprite 0 uses tile 4 (opaque at both (0,7) and (2,0)), at screen position (0,0).
    const auto writeOam = [&](std::uint8_t addr, std::uint8_t data) {
        nes.cpuBusWrite(0x2003, addr);
        nes.cpuBusWrite(0x2004, data);
    };
    writeOam(0x00, 0x00);  // Y = 0
    writeOam(0x01, 0x04);  // tile = 4
    writeOam(0x02, 0x00);  // attributes = 0 (in front of background)
    writeOam(0x03, 0x00);  // X = 0

    nes.cpuBusWrite(0x2000, 0x00);  // background pattern table $0000, nametable 0

    nes.cpuBusWrite(0x2001, 0x08);  // background only
    const auto bgOnly = nes.ppu.renderFrame();

    nes.cpuBusWrite(0x2001, 0x18);  // background + sprites, sprite in front
    const auto inFront = nes.ppu.renderFrame();

    writeOam(0x02, 0x20);  // set the background-priority bit
    const auto behind = nes.ppu.renderFrame();

    // (0,7): background transparent -> sprite is visible in front and behind
    REQUIRE(!eq(inFront[0][7], bgOnly[0][7]));
    REQUIRE(eq(behind[0][7], inFront[0][7]));

    // (2,0): background opaque -> sprite is visible in front but hidden behind
    REQUIRE(!eq(inFront[2][0], bgOnly[2][0]));
    REQUIRE(eq(behind[2][0], bgOnly[2][0]));
}
