#include "ppu/Ntsc2C02.hpp"
#include "nes_core/Nes.hpp"

namespace NES {

NTSC2C02::NTSC2C02(NesSystem& nes) : sys(nes) {
    reset();
}

void NTSC2C02::cycle() {
    // Reference: https://www.nesdev.org/wiki/PPU_rendering

    static constexpr int CYCLES_PER_SCANLINE = 341;
    static constexpr int SCANLINES_PER_FRAME = 262;

    cycles++;

    if (scanline == -1) [[unlikely]] {
        frameCompleted = false;
    }

    if (cycles >= CYCLES_PER_SCANLINE) [[unlikely]] {
        cycles = 0;
        scanline++;

        if (scanline == 241) [[unlikely]] {  // 241
            r_PPUSTATUS[VerticalBlanking] = true;
            if (r_PPUCTRL[GenerateNMI]) {
                sys.generateNmi();
            }
        }

        // TODO refactor this part
        if (scanline >= SCANLINES_PER_FRAME - 1) [[unlikely]] {  // 261
            scanline = -1;
            frameCompleted = true;
            r_PPUSTATUS[VerticalBlanking] = false;
            // TODO sprite 0
            // TODO overflow
        }
    }
}

void NTSC2C02::reset() noexcept {
    cycles = 0;
    scanline = 0;
    frameCompleted = false;
    r_PPUCTRL = 0;
    r_PPUMASK = 0;
    r_PPUSTATUS = 0;
    // r_OAMADDR = 0;
    // r_OAMDATA = 0;
    r_PPUSCROLL = 0;
    r_PPUADDR = 0;
    r_PPUDATA = 0;
}

void NTSC2C02::controllerWrite(std::uint8_t data) noexcept {
    r_PPUCTRL = data;
}

void NTSC2C02::maskWrite(std::uint8_t data) noexcept {
    r_PPUMASK = data;
}

std::uint8_t NTSC2C02::statusRead() noexcept {
    auto status = static_cast<uint8_t>(r_PPUSTATUS.to_ulong() & 0xE0);
    // add some noise to the lower 5 bits
    status |= static_cast<uint8_t>(dataBuffer & 0x1F);

    r_PPUSTATUS[VerticalBlanking] = false;  // reset VBlank flag
    waitingLowByte = false;                 // reset address latch

    return status;  // adds some noise to the lower 5 bits
}

void NTSC2C02::scrollWrite(std::uint8_t data) noexcept {
    // TODO
    data++;
}

void NTSC2C02::addressWrite(std::uint8_t data) noexcept {
    if (!waitingLowByte) {
        r_PPUADDR = static_cast<uint16_t>((r_PPUADDR & 0x00FF) | (data << 8));
    } else {
        r_PPUADDR = (r_PPUADDR & 0xFF00) | data;
    }
    waitingLowByte = !waitingLowByte;
}

void NTSC2C02::dataWrite(std::uint8_t data) noexcept {
    sys.ppuBusWrite(r_PPUADDR, data);

    r_PPUADDR += r_PPUCTRL[IncrementMode] ? static_cast<uint16_t>(32) : static_cast<uint16_t>(1);
}

std::uint8_t NTSC2C02::dataRead() noexcept {
    // Reads to CHR ROM / VRAM are delayed, so they are served from a buffer
    auto data = dataBuffer;
    dataBuffer = sys.ppuBusRead(r_PPUADDR);  // TODO this can be optimized

    // Increment address based on the PPUCTRL register
    r_PPUADDR += r_PPUCTRL[IncrementMode] ? static_cast<uint16_t>(32) : static_cast<uint16_t>(1);

    // For palette RAM this access is instantaneous
    if (r_PPUADDR >= 0x3F00) {
        data = dataBuffer;
    }
    return data;
}

Image<Color, 256, 240> NTSC2C02::renderFrame() const noexcept {
    return Image<Color, 256, 240>();
}

Image<Color, 256, 240> NTSC2C02::renderBackground() const noexcept {
    Image<Color, 256, 240> bg;

    std::array<std::array<NES::Color, 4>, 4> bgPalette = {
        sys.getPalette(0x00),
        sys.getPalette(0x01),
        sys.getPalette(0x02),
        sys.getPalette(0x03),
    };

    // TODO review later
    size_t nametable = static_cast<size_t>(((r_PPUCTRL[NametableIdx2] << 11) | r_PPUCTRL[NametableIdx1]) << 10);
    // 0010 0000 0000 0000 - 0x2000
    // 0010 0100 0000 0000 - 0x2400
    // 0010 1000 0000 0000 - 0x2800
    // 0010 1100 0000 0000 - 0x2C00

    for (size_t nt_y = 0; nt_y < 30; nt_y++) {
        for (size_t nt_x = 0; nt_x < 32; nt_x++) {

            int position = static_cast<int>(nametable + nt_y * 32 + nt_x);
            Image<Palette, 8, 8> sprite = sys.getSprite(sys.vram.read(static_cast<uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            uint8_t attribute = sys.vram.read(static_cast<uint16_t>(nametable + 32 * 30 + (nt_y >> 2) * 8 + (nt_x >> 2)));
            size_t palette = (attribute >> (((nt_y & 0x02) + ((nt_x & 0x02) >> 1)) * 2)) & 0x03;

            for (size_t y = 0; y < 8; y++) {
                for (size_t x = 0; x < 8; x++) {
                    bg[nt_y * 8 + y][nt_x * 8 + x] = bgPalette[palette][sprite[y][x]];
                }
            }
        }
    }

    return bg;
}

// TODO: refactor this methods later
Image<Color, 256, 240> NTSC2C02::renderNametable1() const noexcept {
    Image<Color, 256, 240> bg;

    std::array<std::array<NES::Color, 4>, 4> bgPalette = {
        sys.getPalette(0x00),
        sys.getPalette(0x01),
        sys.getPalette(0x02),
        sys.getPalette(0x03),
    };

    size_t nametable = 0x2000;

    for (size_t nt_y = 0; nt_y < 30; nt_y++) {
        for (size_t nt_x = 0; nt_x < 32; nt_x++) {

            int position = static_cast<int>(nametable + nt_y * 32 + nt_x);
            Image<Palette, 8, 8> sprite = sys.getSprite(sys.vram.read(static_cast<uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            uint8_t attribute = sys.vram.read(static_cast<uint16_t>(nametable + 32 * 30 + (nt_y >> 2) * 8 + (nt_x >> 2)));
            size_t palette = (attribute >> (((nt_y & 0x02) + ((nt_x & 0x02) >> 1)) * 2)) & 0x03;

            for (size_t y = 0; y < 8; y++) {
                for (size_t x = 0; x < 8; x++) {
                    bg[nt_y * 8 + y][nt_x * 8 + x] = bgPalette[palette][sprite[y][x]];
                }
            }
        }
    }

    return bg;
}

Image<Color, 256, 240> NTSC2C02::renderNametable2() const noexcept {
    Image<Color, 256, 240> bg;

    std::array<std::array<NES::Color, 4>, 4> bgPalette = {
        sys.getPalette(0x00),
        sys.getPalette(0x01),
        sys.getPalette(0x02),
        sys.getPalette(0x03),
    };

    size_t nametable = 0x2C00;

    for (size_t nt_y = 0; nt_y < 30; nt_y++) {
        for (size_t nt_x = 0; nt_x < 32; nt_x++) {

            int position = static_cast<int>(nametable + nt_y * 32 + nt_x);
            Image<Palette, 8, 8> sprite = sys.getSprite(sys.vram.read(static_cast<uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            uint8_t attribute = sys.vram.read(static_cast<uint16_t>(nametable + 32 * 30 + (nt_y >> 2) * 8 + (nt_x >> 2)));
            size_t palette = (attribute >> (((nt_y & 0x02) + ((nt_x & 0x02) >> 1)) * 2)) & 0x03;

            for (size_t y = 0; y < 8; y++) {
                for (size_t x = 0; x < 8; x++) {
                    bg[nt_y * 8 + y][nt_x * 8 + x] = bgPalette[palette][sprite[y][x]];
                }
            }
        }
    }

    return bg;
}

Image<Color, 256, 240> NTSC2C02::renderForeground() const noexcept {
    //    std::array<std::array<NES::Color, 4>, 4> fgPalette = {
    //        sys.getPalette(0x04),
    //        sys.getPalette(0x05),
    //        sys.getPalette(0x06),
    //        sys.getPalette(0x07),
    //    };

    // use r_PPUCTRL[BackgroundTable] to know which table use (0 for 0x0000, 1 for 0x1000)
    return Image<Color, 256, 240>();
}

Image<Color, 128, 128> NTSC2C02::renderPatternTable(uint8_t table, uint8_t palette) const noexcept {
    constexpr size_t TILE_SIZE = 8;
    constexpr size_t GRID_TILE_SIZE = 16;

    Image<Color, 128, 128> pattern;

    if (palette > 0x03) {
        // palette = nes.getPPU().getPalette();
    }
    std::array<NES::Color, 4> colorPalette = sys.getPalette(palette);  // TODO get palette from PPU

    for (size_t tile_y = 0; tile_y < GRID_TILE_SIZE; tile_y++) {
        for (size_t tile_x = 0; tile_x < GRID_TILE_SIZE; tile_x++) {

            Image<Palette, 8, 8> data = sys.getSprite(static_cast<uint8_t>(tile_y), static_cast<uint8_t>(tile_x), table);

            for (size_t pixel_y = 0; pixel_y < TILE_SIZE; pixel_y++) {
                for (size_t pixel_x = 0; pixel_x < TILE_SIZE; pixel_x++) {
                    NES::Color c = colorPalette[data[pixel_y][pixel_x]];
                    pattern[tile_y * TILE_SIZE + pixel_y][tile_x * TILE_SIZE + pixel_x] = c;
                }
            }
        }
    }

    return pattern;
}

}  // namespace NES