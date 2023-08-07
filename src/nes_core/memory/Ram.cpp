#include "Ram.hpp"

namespace NES {

Ram::Ram(Bus& bus, std::uint16_t startAddr, std::uint16_t endAddr) : BusSubscriber(startAddr, endAddr), memory(endAddr - startAddr + 1, 0) {
    bus.addDevice(this);
}

void Ram::writeHandler(std::uint16_t addr, std::uint8_t data) {
    memory[addr] = data;
}

std::uint8_t Ram::readHandler(std::uint16_t addr) {
    return memory[addr];
}

}  // namespace NES
