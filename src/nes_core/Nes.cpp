#include "nes_core/Nes.hpp"
#include <iostream>

namespace NES {

NesSystem::NesSystem(const iNES::iNES& rom) : vram(rom.mirroring), gpak(rom), cpu(*this), ppu(*this) {
    reset();
}

NesSystem::~NesSystem() = default;

void NesSystem::reset() noexcept {
    clockCounter = 0;
    pendingNmi = false;
    cpu.reset();
}

void NesSystem::cycle() noexcept {
    ppu.cycle();
    if ((clockCounter & 0x03) == 0) {
        cpu.cycle();
    }

    if (pendingNmi) [[unlikely]] {
        cpu.nmi();
        pendingNmi = false;
    }

    clockCounter++;
}

void NesSystem::runUntilFrame() noexcept {
    do {
        cycle();
    } while (!ppu.isFrameCompleted());
}

bool NesSystem::isRunning() {
    return cpu.getOpcode() != 0x00;
}

void NesSystem::setPC(std::uint16_t pc) noexcept {
    cpu.setPC(pc);
}

std::uint16_t NesSystem::getPC() const noexcept {
    return cpu.getPC();
}

std::uint64_t NesSystem::getCycles() const noexcept {
    return cpu.getCycles();
}

std::uint64_t NesSystem::getInstructions() const noexcept {
    return cpu.getInstructions();
}

Image<Palette, 8, 8> NesSystem::getSprite(std::uint8_t tile, bool rightTable) const noexcept {
    return gpak.getSprite(tile, rightTable);
}

Image<Palette, 8, 8> NesSystem::getSprite(std::uint8_t tileI, std::uint8_t tileJ, bool rightTable) const noexcept {
    return getSprite(static_cast<std::uint8_t>(tileI * 16 + tileJ), rightTable);
}

Color NesSystem::getColor(std::uint8_t palette, std::uint8_t color) const noexcept {
    return paletteRam.getColor(palette, color);
}

std::array<Color, 4> NesSystem::getPalette(std::uint8_t palette) const noexcept {
    return paletteRam.getPalette(palette);
}

void NesSystem::cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/CPU_memory_map

    if (addr >= CPU_RAM_START && addr <= CPU_RAM_END) {
        ram.write(addr, data);

    } else if (addr >= CPU_PPU_START && addr <= CPU_PPU_END) {
        // Reference: https://www.nesdev.org/wiki/PPU_registers
        switch (addr) {
            case CPU_PPU_START:
                ppu.controllerWrite(data);
                break;
            case CPU_PPU_START + 1:
                ppu.maskWrite(data);
                break;
            case CPU_PPU_START + 2:
                // TODO Status
                break;
            case CPU_PPU_START + 3:
                // TODO OAM Address
                break;
            case CPU_PPU_START + 4:
                // TODO OAM Data
                break;
            case CPU_PPU_START + 5:
                ppu.scrollWrite(data);
                break;
            case CPU_PPU_START + 6:
                ppu.addressWrite(data);
                break;
            case CPU_PPU_START + 7:
                ppu.dataWrite(data);
                break;
            default:
                break;
        }

    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: write to APU

    } else if (addr >= CPU_CARTRIDGE_START && addr <= CPU_CARTRIDGE_END) {
        gpak.prgRomWrite(addr, data);
    }
}

std::uint8_t NesSystem::cpuBusRead(std::uint16_t addr) noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/CPU_memory_map

    if (addr >= CPU_RAM_START && addr <= CPU_RAM_END) {
        return ram.read(addr);

    } else if (addr >= CPU_PPU_START && addr <= CPU_PPU_END) {
        // Reference: https://www.nesdev.org/wiki/PPU_registers

        switch (addr) {
            case CPU_PPU_START + 2:
                return ppu.statusRead();
            case CPU_PPU_START + 7:
                return ppu.dataRead();
            default:
                break;
        }

    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: read from APU

    } else if (addr >= CPU_CARTRIDGE_START && addr <= CPU_CARTRIDGE_END) {
        return gpak.prgRomRead(addr);
    }

    return 0x00;
}

void NesSystem::ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/PPU_memory_map

    if (addr >= PPU_PATTERN_TABLE_START && addr <= PPU_PATTERN_TABLE_END) {
        gpak.chrRomWrite(addr, data);

    } else if (addr >= PPU_NAME_TABLE_START && addr <= PPU_NAME_TABLE_END) {
        vram.write(addr, data);

    } else if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        paletteRam.write(addr, data);
    }
}

[[nodiscard]] std::uint8_t NesSystem::ppuBusRead(std::uint16_t addr) const noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/PPU_memory_map

    if (addr >= PPU_PATTERN_TABLE_START && addr <= PPU_PATTERN_TABLE_END) {
        return gpak.chrRomRead(addr);

    } else if (addr >= PPU_NAME_TABLE_START && addr <= PPU_NAME_TABLE_END) {
        return vram.read(addr);

    } else if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        return paletteRam.read(addr);
    }
    return 0x00;
}

}  // namespace NES
