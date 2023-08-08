#ifndef NES_EMULATOR_INES_HPP
#define NES_EMULATOR_INES_HPP

#include <string>
#include <vector>

namespace iNES {

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
