#ifndef NES_EMULATOR_MAPPER_000_HPP
#define NES_EMULATOR_MAPPER_000_HPP

#include <cassert>
#include "cartridge/mapper/Mapper.hpp"

namespace NES {

// TODO: write documentation/put reference
// PrgRom always is mapped by address from 0x8000 to 0xFFFF (so it will require a AND operation)
// ChrRom always is mapped by address from 0x0000 to 0x1FFF

class Mapper_000 : public Mapper {
  public:
    Mapper_000(std::uint8_t nPrgRomBanks, std::uint8_t nChrRomBanks) : Mapper(nPrgRomBanks, nChrRomBanks) {}

    ~Mapper_000() override = default;

    [[nodiscard]] std::uint32_t mapPrgRomRead(std::uint16_t addr) const noexcept override {
        // PRG ROM ranges from 0x8000 to 0xFFFF for the CPU
        return addr & (nPrgRomBanks > 1 ? 0x7FFF : 0x3FFF);
    }

    [[nodiscard]] std::uint32_t mapPrgRomWrite([[maybe_unused]] std::uint16_t addr) const noexcept override {
        assert(false && "PRG ROM write not supported");
        return 0x00;
    }

    [[nodiscard]] std::uint32_t mapChrRomRead(std::uint16_t addr) const noexcept override {
        // CHR ROM ranges from 0x0000 to 0x1FFF for the PPU
        return addr;
    }

    [[nodiscard]] std::uint32_t mapChrRomWrite([[maybe_unused]] std::uint16_t addr) const noexcept override {
        assert(false && "CHR ROM write not supported");
        return 0x00;
    }
};

}  // namespace NES

#endif  //NES_EMULATOR_MAPPER_000_HPP
