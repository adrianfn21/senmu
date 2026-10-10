#include "ppu/Ntsc2C02.hpp"
#include "nes_core/Nes.hpp"

namespace NES {

NTSC2C02::NTSC2C02(NesSystem& nes) : sys(nes), vram(sys.gpak.getMirroring()) {
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

    if (cycles >= CYCLES_PER_SCANLINE) {
        cycles = 0;
        scanline++;

        switch (scanline) {
            case 30:
                // sprite 0 hit: not cycle-accurate, we just set it once if rendering is enabled
                // shortcut: fixed scanline instead of the real overlap, upgrade if needed
                if (r_PPUMASK[RenderBackground] && r_PPUMASK[RenderSprite]) {
                    r_PPUSTATUS[SpriteZero] = true;
                }
                // sprite overflow: same shortcut, just count the sprites sitting on this scanline
                if (r_PPUMASK[RenderSprite] && spritesOnScanline(scanline) >= 8) {
                    r_PPUSTATUS[SpriteOverflow] = true;
                }
                break;

            case 241:
                r_PPUSTATUS[VerticalBlanking] = true;
                if (r_PPUCTRL[GenerateNMI]) {
                    sys.generateNmi();
                }
                break;

            case SCANLINES_PER_FRAME - 1:
                scanline = -1;
                frameCompleted = true;
                frameCounter++;
                r_PPUSTATUS[VerticalBlanking] = false;
                r_PPUSTATUS[SpriteZero] = false;      // cleared during pre-render
                r_PPUSTATUS[SpriteOverflow] = false;  // cleared during pre-render
                break;
        }
    }
}

void NTSC2C02::reset() noexcept {
    cycles = 0;
    scanline = 0;
    frameCounter = 0;
    frameCompleted = false;
    r_PPUCTRL = 0;
    r_PPUMASK = 0;
    r_PPUSTATUS = 0;
    // r_OAMADDR = 0;
    // r_OAMDATA = 0;
    r_PPUADDR = 0;
    r_TMPADDR = 0;
    r_coarseX = 0;
    r_coarseY = 0;
    r_fineX = 0;
    r_fineY = 0;
    r_nametableIdx = 0;
    r_writeToggle = false;
}

int NTSC2C02::spritesOnScanline(int line) const noexcept {
    int count = 0;
    for (int i = 0; i < 64; i++) {
        const std::uint8_t y = oam.read(static_cast<std::uint16_t>(i * 4));  // sprite Y position
        if (y <= line && line < y + 8) count++;                              // 8x8 sprites only (TODO: 8x16)
    }
    return count;
}

void NTSC2C02::controllerWrite(std::uint8_t data) noexcept {
    r_PPUCTRL = data;

    // t: ...GH.. ........ <- d: ......GH (nametable index)
    // Name table index
    // r_TMPADDR = static_cast<std::uint16_t>((r_TMPADDR & 0xF3FF) |  // old content
    //                                   (data & 0x03) << 10     // GH
    // );
    r_nametableIdx = static_cast<std::uint8_t>(data & 0x03);
}

void NTSC2C02::maskWrite(std::uint8_t data) noexcept {
    r_PPUMASK = data;
}

std::uint8_t NTSC2C02::statusRead() noexcept {
    auto status = static_cast<std::uint8_t>(r_PPUSTATUS.to_ulong() & 0xE0);
    // add some noise to the lower 5 bits
    status |= static_cast<std::uint8_t>(dataBuffer & 0x1F);

    r_PPUSTATUS[VerticalBlanking] = false;  // reset VBlank flag
    r_writeToggle = false;                  // reset address latch

    return status;  // adds some noise to the lower 5 bits
}

void NTSC2C02::oamAddrWrite(std::uint8_t data) noexcept {
    r_OAMADDR = data;
}

void NTSC2C02::oamDataWrite(std::uint8_t data) noexcept {
    oam.write(r_OAMADDR, data);
}

std::uint8_t NTSC2C02::oamDataRead() noexcept {
    return oam.read(r_OAMADDR);
}

void NTSC2C02::scrollWrite(std::uint8_t data) noexcept {
    if (!r_writeToggle) {
        // t: ....... ...ABCDE <- d: ABCDE... (write coarse x)
        // r_TMPADDR = static_cast<std::uint16_t>((r_TMPADDR & 0xFFE0) | (data >> 3));
        r_coarseX = static_cast<std::uint8_t>(data >> 3);

        // x:              FGH <- d: .....FGH (write fine x)
        r_fineX = static_cast<std::uint8_t>(data & 0x07);
    } else {
        // t: FGH..AB CDE..... <- d: ABCDEFGH (write coarse y and fine y)
        // r_TMPADDR = static_cast<std::uint16_t>((r_TMPADDR & 0x0C1F) |   // old content
        //                                   ((data & 0x07) << 12) |  // FGH
        //                                   ((data & 0xF8) << 2)     // ABCDE
        // );
        r_coarseY = static_cast<std::uint8_t>((data & 0xF8) >> 3);
        r_fineY = static_cast<std::uint8_t>(data & 0x07);
    }

    r_writeToggle = !r_writeToggle;
}

void NTSC2C02::addressWrite(std::uint8_t data) noexcept {
    if (!r_writeToggle) {
        // t: ZCDEFGH ........ <- d: ..CDEFGH (Z is cleared)
        r_TMPADDR = static_cast<std::uint16_t>((r_TMPADDR & 0xFF) |  // old content + Z cleared
                                               ((data & 0x3F) << 8)  // CDEFGH
        );
    } else {
        // t: ....... ABCDEFGH <- d: ABCDEFGH
        r_TMPADDR = static_cast<std::uint16_t>((r_TMPADDR & 0x7F00) |  // old content
                                               data                    // ABCDEFGH
        );

        // v: <...all bits...> <- t: <...all bits...>
        r_PPUADDR = r_TMPADDR;
    }
    r_writeToggle = !r_writeToggle;
}

void NTSC2C02::dataWrite(std::uint8_t data) noexcept {
    ppuBusWrite(r_PPUADDR, data);
    r_PPUADDR += r_PPUCTRL[IncrementMode] ? static_cast<std::uint16_t>(32) : static_cast<std::uint16_t>(1);
}

std::uint8_t NTSC2C02::dataRead() noexcept {
    // Reads to CHR ROM / VRAM are delayed, so they are served from a buffer
    auto data = dataBuffer;
    dataBuffer = ppuBusRead(r_PPUADDR);  // TODO this can be optimized

    // For palette RAM this access is instantaneous
    if (r_PPUADDR >= 0x3F00) {
        data = dataBuffer;
    }

    // Increment address based on the PPUCTRL register
    r_PPUADDR += r_PPUCTRL[IncrementMode] ? static_cast<std::uint16_t>(32) : static_cast<std::uint16_t>(1);

    return data;
}

void NTSC2C02::ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/PPU_memory_map

    if (addr >= PPU_PATTERN_TABLE_START && addr <= PPU_PATTERN_TABLE_END) {
        sys.gpak.chrRomWrite(addr, data);

    } else if (addr >= PPU_NAME_TABLE_START && addr <= PPU_NAME_TABLE_END) {
        vram.write(addr, data);

    } else if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        paletteRam.write(addr, data);
    }
}

[[nodiscard]] std::uint8_t NTSC2C02::ppuBusRead(std::uint16_t addr) const noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/PPU_memory_map

    if (addr >= PPU_PATTERN_TABLE_START && addr <= PPU_PATTERN_TABLE_END) {
        return sys.gpak.chrRomRead(addr);

    } else if (addr >= PPU_NAME_TABLE_START && addr <= PPU_NAME_TABLE_END) {
        return vram.read(addr);

    } else if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        return paletteRam.read(addr);
    }
    return 0x00;
}

Image<Color, 256, 240> NTSC2C02::renderFrame() const noexcept {
    Image<Color, 256, 240> img{};
    BackgroundMask bgOpaque{};

    const bool background = r_PPUMASK[RenderBackground];

    if (background)
        img = renderBackground(&bgOpaque);

    if (r_PPUMASK[RenderSprite])
        renderForeground(img, background ? &bgOpaque : nullptr);

    return img;
}

Image<Color, 256, 240> NTSC2C02::renderBackground(BackgroundMask* opaque) const noexcept {
    // TODO: optimize later

    Image<Color, 256, 240> bg;

    std::array<std::array<NES::Color, 4>, 4> bgPalette = {
        getPalette(0x00),
        getPalette(0x01),
        getPalette(0x02),
        getPalette(0x03),
    };

    // TODO review later
    size_t original_nametable = static_cast<std::uint16_t>(0x2000 + (r_nametableIdx << 10));
    // 0010 0000 0000 0000 - 0x2000
    // 0010 0100 0000 0000 - 0x2400
    // 0010 1000 0000 0000 - 0x2800
    // 0010 1100 0000 0000 - 0x2C00

    // The visible area needs an extra tile row/column to cover the fine-scroll offset.
    for (size_t nt_y = static_cast<size_t>(r_coarseY); nt_y < 31 + static_cast<size_t>(r_coarseY); nt_y++) {
        size_t y_nametable = original_nametable;

        if (nt_y >= 30) {
            if (((y_nametable >> 11) & 0x01) == 0) {
                y_nametable = original_nametable + 0x800;
            } else {
                y_nametable = original_nametable - 0x800;
            }
        }

        for (size_t nt_x = static_cast<size_t>(r_coarseX); nt_x < 33 + static_cast<size_t>(r_coarseX); nt_x++) {
            size_t xy_nametable = y_nametable;

            if (nt_x >= 32) {
                if ((xy_nametable >> 10) & 0x01) {
                    xy_nametable = original_nametable + 0x400;
                } else {
                    xy_nametable = original_nametable - 0x400;
                }
            }

            std::uint16_t new_y = static_cast<std::uint16_t>(nt_y % 30);
            std::uint16_t new_x = static_cast<std::uint16_t>(nt_x % 32);

            //            if (nt_x == 0) {
            //                std::cout << std::hex << "ori_nametable:" << original_nametable << "(" << r_PPUCTRL[NametableIdx2] << "," << r_PPUCTRL[NametableIdx1]
            //                          << ")  nametable:" << nametable << std::endl;
            //                std::cout << std::dec << "nt_x=" << nt_x << " nt_y=" << nt_y << "   new_x=" << new_x << " new_y=" << new_y << std::endl;
            //            }

            int position = static_cast<int>(xy_nametable + new_y * 32 + new_x);
            Image<Palette, 8, 8> sprite = getSprite(vram.read(static_cast<std::uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            std::uint8_t attribute = vram.read(static_cast<std::uint16_t>(xy_nametable + 32 * 30 + (new_y >> 2) * 8 + (new_x >> 2)));
            size_t palette = (attribute >> (((new_y & 0x02) + ((new_x & 0x02) >> 1)) * 2)) & 0x03;

            // Fine scroll shifts the whole background up/left by 0-7 pixels.
            for (size_t y = 0; y < 8; y++) {
                for (size_t x = 0; x < 8; x++) {
                    const int screenX = static_cast<int>((nt_x - r_coarseX) * 8 + x) - r_fineX;
                    const int screenY = static_cast<int>((nt_y - r_coarseY) * 8 + y) - r_fineY;
                    if (screenX < 0 || screenX >= 256 || screenY < 0 || screenY >= 240)
                        continue;  // pixel falls outside the visible area
                    bg[static_cast<std::size_t>(screenY)][static_cast<std::size_t>(screenX)] = bgPalette[palette][sprite[y][x]];
                    if (opaque != nullptr)
                        (*opaque)[static_cast<std::size_t>(screenY) * 256 + static_cast<std::size_t>(screenX)] = sprite[y][x] != 0;
                }
            }
        }
    }

    return bg;
}

void NTSC2C02::renderForeground(Image<Color, 256, 240>& img, const BackgroundMask* bgOpaque) const noexcept {
    std::array<std::array<NES::Color, 4>, 4> fgPalette = {
        getPalette(0x04),
        getPalette(0x05),
        getPalette(0x06),
        getPalette(0x07),
    };

    // Only the first 8 sprites (in OAM order) are drawn on each scanline.
    std::array<std::array<bool, 64>, 240> onScanline{};
    for (std::size_t line = 0; line < 240; line++) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < 64 && count < 8; i++) {
            const std::uint8_t startY = oam.read(static_cast<std::uint16_t>(i * 4 + 0));
            if (line >= startY && line < static_cast<std::size_t>(startY) + 8) {
                onScanline[line][i] = true;
                count++;
            }
        }
    }

    // Lower indexes have higher priority, so we render sprites from the end of OAM to the beginning
    for (std::size_t i = 63; i <= 64; i--) {  // TODO: variable num of sprites?
        std::uint8_t startY = oam.read(static_cast<std::uint16_t>(i * 4 + 0));
        std::uint8_t tile = oam.read(static_cast<std::uint16_t>(i * 4 + 1));
        std::uint8_t attr = oam.read(static_cast<std::uint16_t>(i * 4 + 2));
        std::uint8_t startX = oam.read(static_cast<std::uint16_t>(i * 4 + 3));

        // TODO: 8x16 mode not supported yet
        if (r_PPUCTRL[Size])
            assert(false && "8x16 sprites not supported yet");

        // TODO: right table not supported
        Image<Palette, 8, 8> sprite = getSprite(tile, false);
        std::uint8_t palette = (attr & 0x03);  // TODO: take care, +0x04 was previously added

        //        for (std::size_t y = 0; y < size_t(std::min(8, 230 - startY)); y++) {
        //            for (std::size_t x = 0; x < size_t(std::min(8, 240 - startX)); x++) {
        for (std::uint8_t y = 0; y < 8; y++) {
            for (std::uint8_t x = 0; x < 8; x++) {
                if (sprite[y][x] == 0)
                    continue;  // palette 0 is transparent for sprites

                const std::uint16_t finalY = startY + (attr & 0x80 ? (7 - y) : y);  // vertical flip
                const std::uint16_t finalX = startX + (attr & 0x40 ? (7 - x) : x);  // horizontal flip
                if (finalY >= 240 || finalX >= 256)
                    continue;  // sprite's pixel is out of screen vertically

                if (!onScanline[finalY][i])
                    continue;  // this sprite is beyond the 8-sprite limit for the scanline

                // Bit 5 of the attributes puts the sprite behind the background: hide it
                // where the background has an opaque pixel.
                if ((attr & 0x20) && bgOpaque != nullptr && (*bgOpaque)[static_cast<std::size_t>(finalY) * 256 + finalX])
                    continue;

                img[finalY][finalX] = fgPalette[palette][sprite[y][x]];
            }
        }
    }
}

// TODO: refactor this methods later
Image<Color, 256, 240> NTSC2C02::renderNametable1() const noexcept {
    Image<Color, 256, 240> bg;

    std::array<std::array<NES::Color, 4>, 4> bgPalette = {
        getPalette(0x00),
        getPalette(0x01),
        getPalette(0x02),
        getPalette(0x03),
    };

    size_t nametable = 0x2000;

    for (size_t nt_y = 0; nt_y < 30; nt_y++) {
        for (size_t nt_x = 0; nt_x < 32; nt_x++) {

            int position = static_cast<int>(nametable + nt_y * 32 + nt_x);
            Image<Palette, 8, 8> sprite = getSprite(vram.read(static_cast<std::uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            std::uint8_t attribute = vram.read(static_cast<std::uint16_t>(nametable + 32 * 30 + (nt_y >> 2) * 8 + (nt_x >> 2)));
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
        getPalette(0x00),
        getPalette(0x01),
        getPalette(0x02),
        getPalette(0x03),
    };

    size_t nametable = 0x2C00;

    for (size_t nt_y = 0; nt_y < 30; nt_y++) {
        for (size_t nt_x = 0; nt_x < 32; nt_x++) {

            int position = static_cast<int>(nametable + nt_y * 32 + nt_x);
            Image<Palette, 8, 8> sprite = getSprite(vram.read(static_cast<std::uint16_t>(position)), r_PPUCTRL[BackgroundTable]);

            std::uint8_t attribute = vram.read(static_cast<std::uint16_t>(nametable + 32 * 30 + (nt_y >> 2) * 8 + (nt_x >> 2)));
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
    //        getPalette(0x04),
    //        getPalette(0x05),
    //        getPalette(0x06),
    //        getPalette(0x07),
    //    };

    // use r_PPUCTRL[BackgroundTable] to know which table use (0 for 0x0000, 1 for 0x1000)
    return Image<Color, 256, 240>();
}

Image<Color, 128, 128> NTSC2C02::renderPatternTable(std::uint8_t table, std::uint8_t palette) const noexcept {
    constexpr size_t TILE_SIZE = 8;
    constexpr size_t GRID_TILE_SIZE = 16;

    Image<Color, 128, 128> pattern;

    if (palette > 0x03) {
        // palette = nes.getPPU().getPalette();
    }
    std::array<NES::Color, 4> colorPalette = getPalette(palette);  // TODO get palette from PPU

    for (size_t tile_y = 0; tile_y < GRID_TILE_SIZE; tile_y++) {
        for (size_t tile_x = 0; tile_x < GRID_TILE_SIZE; tile_x++) {

            Image<Palette, 8, 8> data = getSprite(static_cast<std::uint8_t>(tile_y), static_cast<std::uint8_t>(tile_x), table);

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

Image<Palette, 8, 8> NTSC2C02::getSprite(std::uint8_t tile, bool rightTable) const noexcept {
    return sys.gpak.getSprite(tile, rightTable);
}

Image<Palette, 8, 8> NTSC2C02::getSprite(std::uint8_t tileI, std::uint8_t tileJ, bool rightTable) const noexcept {
    return getSprite(static_cast<std::uint8_t>(tileI * 16 + tileJ), rightTable);
}

Color NTSC2C02::getColor(std::uint8_t palette, std::uint8_t color) const noexcept {
    return paletteRam.getColor(palette, color);
}

std::array<Color, 4> NTSC2C02::getPalette(std::uint8_t palette) const noexcept {
    return paletteRam.getPalette(palette);
}

}  // namespace NES