#include "cartridge/GamePak.hpp"

namespace NES {

GamePak::GamePak(const iNES::iNES& rom) : prgRom(rom.prgRom), chrRom(rom.chrRom) {
    switch (rom.mapperNumber()) {
        case 0:
            mapper = std::make_unique<Mapper_000>(rom.header.nPrgRomBanks, rom.header.nChrRomBanks);
            break;
        default:
            assert(false && "Mapper not supported yet");
    }

    // TODO: configure the rest of the parameters
}

void GamePak::prgRomWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    std::uint32_t writeAddr = mapper->mapPrgRomWrite(addr);
    prgRom[writeAddr] = data;
}

std::uint8_t GamePak::prgRomRead(std::uint16_t addr) const noexcept {
    std::uint32_t readAddr = mapper->mapPrgRomRead(addr);
    return prgRom[readAddr];
}

void GamePak::chrRomWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    std::uint32_t writeAddr = mapper->mapChrRomWrite(addr);
    chrRom[writeAddr] = data;
}

std::uint8_t GamePak::chrRomRead(std::uint16_t addr) const noexcept {
    std::uint32_t readAddr = mapper->mapChrRomRead(addr);
    return chrRom[readAddr];
}

}  // namespace NES