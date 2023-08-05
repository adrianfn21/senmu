#include <iostream>
#include "nes_core/comm/Bus.hpp"
#include "nes_core/memory/Ram.hpp"

#include <argparse/argparse.hpp>

int main() {
    NES::Bus cpuBus;
    NES::Ram ram(cpuBus, 0x0000, 0x07FF);

    uint8_t originalVal = 5;
    uint16_t addr = 0x8000;

    cpuBus.write(addr, originalVal);
    uint8_t val = cpuBus.read(addr);

    std::cout << (int)originalVal << " " << (int)val << std::endl;
}