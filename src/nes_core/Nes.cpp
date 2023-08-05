#include "Nes.hpp"

namespace NES {

NesSystem::NesSystem()
    : cpuBus(),
      ram(cpuBus, 0x0000, 0xFFFF),  // TODO: change later to 0x07FF
      cpu(cpuBus) {
    reset();
}

NesSystem::~NesSystem() = default;

void NesSystem::reset() {
    cpu.reset();
}

void NesSystem::cycle() {
    cpu.cycle();
}

}  // namespace NES
