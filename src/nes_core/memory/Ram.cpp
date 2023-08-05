#include "Ram.hpp"

namespace NES {

Ram::Ram(Bus& bus, uint16_t startAddr, uint16_t endAddr) : BusSubscriber(startAddr, endAddr) {
    bus.addDevice(this);
    memory.fill(0);
}

void Ram::writeHandler(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

uint8_t Ram::readHandler(uint16_t addr) {
    return memory[addr];
}

}  // namespace NES
