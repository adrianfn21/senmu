#include "nes_core/Nes.hpp"
#include <iostream>

namespace NES {

NesSystem::NesSystem(const iNES::iNES& rom) : gpak(rom), cpu(*this) {
    reset();
}

NesSystem::~NesSystem() = default;

void NesSystem::reset() noexcept {
    cpu.reset();
}

void NesSystem::cycle() noexcept {
    cpu.cycle();
}

void NesSystem::step() noexcept {
    cpu.step();
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

uint64_t NesSystem::getCycles() const noexcept {
    return cpu.getCycles();
}

uint64_t NesSystem::getInstructions() const noexcept {
    return cpu.getInstructions();
}

void NesSystem::cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    if (addr >= CPU_RAM_START && addr <= CPU_RAM_END) {
        ram.write(addr, data);
    } else if (addr >= CPU_PPU_START && addr <= CPU_PPU_END) {
        // TODO: write to PPU
    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: write to APU
    } else if (addr >= CPU_CARTRIDGE_START && addr <= CPU_CARTRIDGE_END) {
        gpak.prgRomWrite(addr, data);
    }
}

std::uint8_t NesSystem::cpuBusRead(std::uint16_t addr) const noexcept {
    if (addr >= CPU_RAM_START && addr <= CPU_RAM_END) {
        return ram.read(addr);
    } else if (addr >= CPU_PPU_START && addr <= CPU_PPU_END) {
        // TODO: read from PPU
    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: read from APU
    } else if (addr >= CPU_CARTRIDGE_START && addr <= CPU_CARTRIDGE_END) {
        return gpak.prgRomRead(addr);
    }

    return 0x00;
}

void NesSystem::ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        // TODO: read from ROM
    } else if (addr >= PPU_NAME_TABLE_START && addr <= PPU_NAME_TABLE_END) {
        // TODO: read from PPU VRAM
    } else if (addr >= PPU_PALETTE_START && addr <= PPU_PALETTE_END) {
        // TODO: read from PPU Palette
    }

    data++;  // TODO: remove this
}

[[nodiscard]] std::uint8_t NesSystem::ppuBusRead(std::uint16_t addr) const noexcept {
    addr++;  // TODO remove this
    return 0x00;
}

}  // namespace NES
