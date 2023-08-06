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

void NesSystem::setPC(uint16_t pc) noexcept {
    cpu.setPC(pc);
}

uint16_t NesSystem::getPC() const noexcept {
    return cpu.getPC();
}

uint64_t NesSystem::getCycles() const noexcept {
    return cpu.getCycles();
}

void NesSystem::loadRom(const std::vector<uint8_t>& romData) {
    if (romData.size() > ROM_SIZE) {
        std::cout << "ROM size is too big" << std::endl;
        exit(1);
    }

    for (size_t i = 0; i < static_cast<uint16_t>(romData.size()); i++) {
        cpuBus.write(ROM_START + i, romData[i]);
    }
}

void NesSystem::write(uint16_t addr, uint8_t data) const noexcept {
    cpuBus.write(addr, data);
}

uint8_t NesSystem::read(uint16_t addr) const noexcept {
    return cpuBus.read(addr);
}

}  // namespace NES
