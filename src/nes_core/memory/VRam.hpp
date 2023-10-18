#ifndef NES_EMULATOR_VRAM_HPP
#define NES_EMULATOR_VRAM_HPP

#include <cassert>
#include "ines/iNes.hpp"
#include "memory/Ram.hpp"

namespace NES {

/**
 * @brief Video RAM
 *
 * The NES has 2 KB of video RAM, which is used to store the nametables and the attribute
 * tables. Each nametable uses 1 KB of memory. The VRAM has a 4KB address space, but only
 * 2KB are used. The remaining space is used for mirroring, which is defined by the game's
 * iNES header.
 *
 * The nametables are used to store the tiles that will be rendered on the screen. Each
 * tile is 8x8 pixels, and each nametable can store 30x32 tiles. The attribute tables
 * are used to store the palette of each tile. Each tile uses 2 bits to store the palette
 * index, so each attribute table can store 64 tiles.
 *
 * @see Memory mapping: https://www.nesdev.org/wiki/PPU_memory_map
 * @see Nametables structure: https://www.nesdev.org/wiki/PPU_nametables
 * @see Attribute table structure: https://www.nesdev.org/wiki/PPU_attribute_tables
 * @see Mirroring modes of nametables: https://www.nesdev.org/wiki/Mirroring
 */
class VRam : private Ram<2 * 1024> {
  public:
    /**
     * @brief VRAM constructor.
     *
     * Currently only horizontal and vertical mirroring are supported.
     *
     * @param mirroring The mirroring type to use.
     */
    VRam(iNES::NtMirroring mirroring) : mirroring(mirroring) { assert(mirroring == iNES::NtMirroring::HORIZONTAL || mirroring == iNES::NtMirroring::VERTICAL); }

    /**
     * @brief VRAM destructor.
     */
    ~VRam() = default;

  public:  // Memory operations
    /**
     * @brief Write to the VRAM.
     *
     * Applies the corresponding mirroring before writing.
     *
     * @param addr Address to write to
     * @param data Data to write
     */
    constexpr void write(std::uint16_t addr, std::uint8_t data) noexcept { Ram::write(mirror(addr), data); }

    /**
     * @brief Read from the VRAM.
     *
     * Applies the corresponding mirroring before reading.
     *
     * @param addr Address to read from
     * @return Data at addr
     */
    [[nodiscard]] constexpr std::uint8_t read(std::uint16_t addr) const noexcept { return Ram::read(mirror(addr)); }

  private:  // Helper properties and methods for memory access
    // Mirroring type (does not change along the execution)
    iNES::NtMirroring mirroring;

    /**
     * @brief Applies mirroring to the address based on the type of mirroring used by the cartridge
     *
     * @param addr Address to mirror
     * @return Mirrored address
     *
     * @see https://www.nesdev.org/wiki/Mirroring
     */
    [[nodiscard]] constexpr std::uint16_t mirror(std::uint16_t addr) const noexcept {
        return static_cast<uint16_t>(
            // Remove nametable index bits
            (addr & ~(0x0800 | 0x0400)) |
            // Add nametable index bits depending on mirroring type
            ((((addr & 0x0800) && !mirroring) || ((addr & 0x0400) && mirroring)) << 10));
    }
};

}  // namespace NES

#endif  //NES_EMULATOR_VRAM_HPP
