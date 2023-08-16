#include "ppu/Ntsc2C02.hpp"
#include "nes_core/Nes.hpp"

namespace NES {

NTSC2C02::NTSC2C02(NesSystem& nes) : sys(nes) {
    reset();
}

void NTSC2C02::cycle() {
    static constexpr int CYCLES_PER_SCANLINE = 341;
    static constexpr int SCANLINES_PER_FRAME = 262;

    cycles++;

    if (scanline == -1) [[unlikely]] {
        frameCompleted = false;
        r_PPUSTATUS[VerticalBlanking] = false;
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

}  // namespace NES