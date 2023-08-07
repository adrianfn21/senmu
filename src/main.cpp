#include <iostream>
#include "nes_core/Nes.hpp"

int main() {
    NES::NesSystem nes;
    std::vector<std::uint8_t> rom = {0x00, 0x01, 0x02, 0x03};
    nes.loadRom(rom);
}