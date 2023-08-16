#ifndef NES_EMULATOR_GAMEPAK_HPP
#define NES_EMULATOR_GAMEPAK_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include "cartridge/mapper/Mapper.hpp"
#include "ines/iNes.hpp"

namespace NES {

// TODO: write documentation

class GamePak {
  public:
    explicit GamePak(const iNES::iNES& rom);

    ~GamePak() = default;

  public:  // Memory access
    // Games must not write to the ROM, but we keep this function for emulation purposes
    void prgRomWrite(std::uint16_t addr, std::uint8_t data) noexcept;

    [[nodiscard]] std::uint8_t prgRomRead(std::uint16_t addr) const noexcept;

    // Games must not write to the ROM, but we keep this function for emulation purposes
    void chrRomWrite(std::uint16_t addr, std::uint8_t data) noexcept;

    [[nodiscard]] std::uint8_t chrRomRead(std::uint16_t addr) const noexcept;

  public:  // Sprite access, abstract away the CHR ROM
    /**
     * @brief Get a sprite from the CHR ROM
     *
     * Abstracts away how the sprites are stored in the CHR ROM, rebuilding a 8x8 array of bytes
     * representing the sprite selected.
     *
     * Sprites are stored in a 16x16 pattern table, so the tile index is a number from 0 to 255.
     * The pattern table is divided in two halves, each one containing 128 tiles. The left table
     * contains the tiles from 0 to 127, while the right table contains the tiles from 128 to 255.
     *
     * @param tile Index of the tile to get (from 0 to 63)
     * @param rightTable If true, use the right table, otherwise use the left table
     *
     * @pre tile >= 0 and tile <= 255
     * @return A 8x8 array of bytes representing the tile, stored in row-major format. Each value represent the value of the palette to use.
     */
    [[nodiscard]] std::array<std::uint8_t, 8 * 8> getSprite(std::uint8_t tile, bool rightTable) const noexcept;

  protected:
    // Program ROM
    std::vector<std::uint8_t> prgRom{};

    // Character ROM
    std::vector<std::uint8_t> chrRom{};

    // Mapper
    std::unique_ptr<Mapper> mapper{};
};

}  // namespace NES

#endif  //NES_EMULATOR_GAMEPAK_HPP
