#ifndef NES_EMULATOR_GAMEPAK_HPP
#define NES_EMULATOR_GAMEPAK_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include "cartridge/mapper/Mapper.hpp"
#include "cartridge/mapper/Mapper_000.hpp"
#include "ines/iNes.hpp"

namespace NES {

// TODO: write documentation

class GamePak {
  public:
    explicit GamePak(const iNES::iNES& rom);

    ~GamePak() = default;

    // Games must not write to the ROM, but we keep this function for emulation purposes
    void prgRomWrite(std::uint16_t addr, std::uint8_t data) noexcept;

    [[nodiscard]] std::uint8_t prgRomRead(std::uint16_t addr) const noexcept;

    // Games must not write to the ROM, but we keep this function for emulation purposes
    void chrRomWrite(std::uint16_t addr, std::uint8_t data) noexcept;

    [[nodiscard]] std::uint8_t chrRomRead(std::uint16_t addr) const noexcept;

  private:
    // Program ROM
    std::vector<std::uint8_t> prgRom{};

    // Character ROM
    std::vector<std::uint8_t> chrRom{};

    // Mapper
    std::unique_ptr<Mapper> mapper{};
};

}  // namespace NES

#endif  //NES_EMULATOR_GAMEPAK_HPP
