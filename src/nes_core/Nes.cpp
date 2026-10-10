#include "nes_core/Nes.hpp"
#include <iostream>

namespace NES {

NesSystem::NesSystem(const iNES::iNES& rom) : gpak(rom), cpu(*this), ppu(*this) {
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
    if (clockCounter % 3 == 0) {
        if (dma.transfer) {
            if (dma.dummy) {
                if (clockCounter % 2 == 1) {
                    dma.dummy = false;
                }
            } else {
                if (clockCounter % 2 == 0) {
                    dma.data = cpuBusRead((static_cast<std::uint16_t>(dma.page) << 8) | dma.addr);
                } else {
                    ppu.oamAddrWrite(dma.addr);
                    ppu.oamDataWrite(dma.data);
                    dma.addr++;

                    if (dma.addr == 0x00) {
                        dma.transfer = false;
                        dma.dummy = true;
                    }
                }
            }
        } else {
            cpu.cycle();
        }
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

bool NesSystem::isRunning() const {
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

void NesSystem::setButton(Controller controllerPort, Button button, bool pressed) noexcept {
    input.setButton(controllerPort, button, pressed);
}

void NesSystem::cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    // TODO: review
    // See for reference: https://www.nesdev.org/wiki/CPU_memory_map

    if (addr >= CPU_RAM_START && addr <= CPU_RAM_END) {
        ram.write(addr, data);

    } else if (addr >= CPU_PPU_START && addr <= CPU_PPU_END) {
        // Reference: https://www.nesdev.org/wiki/PPU_registers
        // Mirrored every 8 bytes
        switch (addr & 0x7) {  // PPU registers
            case 0:            // PPUCTRL
                ppu.controllerWrite(data);
                break;
            case 1:  // PPUMASK
                ppu.maskWrite(data);
                break;
            case 2:  // PPUSTATUS
                // TODO Status
                break;
            case 3:  // OAMADDR
                ppu.oamAddrWrite(data);
                break;
            case 4:  // OAMDATA
                ppu.oamDataWrite(data);
                break;
            case 5:  // PPUSCROLL
                ppu.scrollWrite(data);
                break;
            case 6:  // PPUADDR
                ppu.addressWrite(data);
                break;
            case 7:  // PPUDATA
                ppu.dataWrite(data);
                break;
            default:
                break;
        }
    } else if (addr == CPU_DMA_OAM) {
        dma.write(data);
    } else if (addr >= CPU_CONTROLLER1 && addr <= CPU_CONTROLLER2) {
        input.write(static_cast<std::uint8_t>(addr & 0x01));
    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: write to APU
        // TODO: take care, same address as controller and DMA

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
        // Mirrored every 8 bytes
        switch (addr & 0x7) {
            case 2:  // PPUSTATUS
                return ppu.statusRead();
            case 4:  // OAMDATA
                return ppu.oamDataRead();
            case 7:  // PPUDATA
                return ppu.dataRead();
            default:
                break;
        }
    } else if (addr >= CPU_CONTROLLER1 && addr <= CPU_CONTROLLER2) {
        return input.read(addr & 0x01);
    } else if (addr >= CPU_APU_START && addr <= CPU_APU_END) {
        // TODO: read from APU

    } else if (addr >= CPU_CARTRIDGE_START && addr <= CPU_CARTRIDGE_END) {
        return gpak.prgRomRead(addr);
    }

    return 0x00;
}

}  // namespace NES
