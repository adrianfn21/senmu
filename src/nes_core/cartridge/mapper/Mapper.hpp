#ifndef NES_EMULATOR_MAPPER_HPP
#define NES_EMULATOR_MAPPER_HPP

#include <cstdint>

// TODO: write documentation

class Mapper {
  public:
    Mapper(std::uint8_t nPrgRomBanks, std::uint8_t nChrRomBanks) : nPrgRomBanks(nPrgRomBanks), nChrRomBanks(nChrRomBanks) {}

    virtual ~Mapper() {}

    virtual std::uint32_t mapPrgRomRead(std::uint16_t addr) const noexcept = 0;
    virtual std::uint32_t mapPrgRomWrite(std::uint16_t addr) noexcept = 0;

    virtual std::uint32_t mapChrRomRead(std::uint16_t addr) const noexcept = 0;
    virtual std::uint32_t mapChrRomWrite(std::uint16_t addr) noexcept = 0;

  protected:
    std::uint8_t nPrgRomBanks = 0;
    std::uint8_t nChrRomBanks = 0;
};

#endif  //NES_EMULATOR_MAPPER_HPP
