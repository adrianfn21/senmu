#ifndef NES_EMULATOR_INES_HPP
#define NES_EMULATOR_INES_HPP

#include <string>
#include <vector>

namespace iNES {

/**
 * @brief Nametable Mirroring type
 *
 * This enum represents the getMirroring type of the ROM.
 * It is used to determine the nametable getMirroring type of the PPU.
 */
enum NtMirroring { HORIZONTAL = 0x0, VERTICAL = 0x1, FOUR_SCREEN = 0x02 };

/**
 * @brief iNES format parser
 *
 * This module reads a ROM file in iNES format and parses it.
 * The main fields are stored and are accessible through the
 * public interface.
 *
 * Reference: https://www.nesdev.org/wiki/INES
 */

class iNES {
  public:
    explicit iNES(const std::string& filepath);
    ~iNES() = default;

    /* iNES format fields */
    struct {
        std::uint8_t nPrgRomBanks;
        std::uint8_t nChrRomBanks;
        size_t prgRomSize;
        size_t chrRomSize;
        std::uint8_t flags6;
        std::uint8_t flags7;
        std::uint8_t flags8;
        std::uint8_t flags9;
        std::uint8_t flags10;

        size_t playchoiceInstRomSize;
        size_t playchoicePromSize;
    } header{};

    std::vector<std::uint8_t> prgRom;
    std::vector<std::uint8_t> chrRom;
    std::vector<std::uint8_t> playchoiceInstRom;
    std::vector<std::uint8_t> playchoiceProm;
    std::string title;

    /* Extracted fields from data */
    std::uint8_t mapperNumber;

    NtMirroring mirroring;

  private:
    /* iNES format constants sizes */
    constexpr static size_t HEADER_SIZE = 16;
    constexpr static size_t PRG_ROM_SIZE = 16 * 1024;
    constexpr static size_t CHR_ROM_SIZE = 8 * 1024;
    constexpr static size_t PLAYCHOICE_INST_ROM_SIZE = 8 * 1024;
    constexpr static size_t PLAYCHOICE_PROM_SIZE = 16;
    constexpr static size_t TITLE_SIZE = 128;
};

}  // namespace iNES

#endif  //NES_EMULATOR_INES_HPP
