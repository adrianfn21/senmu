#include "nes_core/Nes.hpp"
#include <iostream>

namespace NES {

NesSystem::NesSystem() : cpuBus(), ram(cpuBus, 0x0000, 0xFFFF), cpu(cpuBus) {
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

void NesSystem::loadRom(const std::vector<std::uint8_t>& romData) {
    if (romData.size() > ROM_SIZE) {
        std::cout << "ROM size is too big" << std::endl;
        exit(1);
    }

    for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(romData.size()); i++) {
        cpuBus.write(ROM_START + i, romData[i]);
    }
}

void NesSystem::write(std::uint16_t addr, std::uint8_t data) const noexcept {
    cpuBus.write(addr, data);
}

std::uint8_t NesSystem::read(std::uint16_t addr) const noexcept {
    return cpuBus.read(addr);
}

}  // namespace NES
