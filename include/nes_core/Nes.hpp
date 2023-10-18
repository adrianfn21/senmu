#ifndef NES_EMULATOR_NES_HPP
#define NES_EMULATOR_NES_HPP

#include <cstdint>
#include <vector>
#include "cartridge/GamePak.hpp"
#include "common/Image.hpp"
#include "cpu/Mos6502.hpp"
#include "memory/PaletteRam.hpp"
#include "memory/Ram.hpp"
#include "memory/VRam.hpp"
#include "ppu/Ntsc2C02.hpp"

namespace NES {

/**
 * @brief The NES system
 *
 * NesSystem is the main class of the emulator. It groups all the elements into a single
 * class and provides the NES Core interface.
 *
 * This class provides methods for loading a ROM, resetting the system, and running it.
 * Also, this class is decoupled from the GUI, so it can be used as a library.
 *
 * It directly integrates the CPU and the PPU, and it is responsible for the communication
 * between them. The buses are simulated just by their respective read and write methods,
 * so both devices can access the memory of the other (but they shouldn't).
 */

class NesSystem {
  public:
    NesSystem(const iNES::iNES& rom);
    ~NesSystem();

    void reset() noexcept;
    void cycle() noexcept;

    void runUntilFrame() noexcept;

    bool isRunning();

    void setPC(std::uint16_t pc) noexcept;
    [[nodiscard]] std::uint16_t getPC() const noexcept;

    [[nodiscard]] std::uint64_t getCycles() const noexcept;
    [[nodiscard]] std::uint64_t getInstructions() const noexcept;

  public:
    [[nodiscard]] Image<Palette, 8, 8> getSprite(std::uint8_t tile, bool rightTable = false) const noexcept;
    [[nodiscard]] Image<Palette, 8, 8> getSprite(std::uint8_t tileI, std::uint8_t tileJ, bool rightTable) const noexcept;

    [[nodiscard]] Color getColor(std::uint8_t palette, std::uint8_t color) const noexcept;
    [[nodiscard]] std::array<Color, 4> getPalette(std::uint8_t palette) const noexcept;

  public:
    void cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t cpuBusRead(std::uint16_t addr) noexcept;

    void ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t ppuBusRead(std::uint16_t addr) const noexcept;

  public:
    /**
     * @brief Generate delayed NMI interrupt.
     *
     * This function is called when the PPU detects the VBLANK flag is set. The NMI
     * interrupt will be generated after the current/next instruction is executed.
     */
    void generateNmi() noexcept { pendingNmi = true; }

  private:
  public:  // TODO REMOVE LATER
    /**
     * @brief CPU RAM
     *
     * The NES has 2 KB of RAM, which is used to store the stack and game data.
     *
     * @see Example of a RAM memory map from a game: https://www.nesdev.org/wiki/Sample_RAM_map
     */
    Ram<2 * 1024> ram;

    /**
     * @brief Video RAM
     *
     * The NES has 2 KB of video RAM, which is used to store the nametables and the attribute
     * tables. Each nametable uses 1 KB of memory. The memory is mirrored horizontally or
     * vertically depending on the mirroring mode of the game.
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
    VRam vram;

    /**
     * @brief Palette RAM
     *
     * The NES has 32 bytes of palette RAM, which is used to store the color palette of the
     * tiles. Each palette has 4 colors, and each color uses 3 bytes to store the RGB values.
     */
    PaletteRam paletteRam;

    GamePak gpak;
    MOS6502 cpu;
    NTSC2C02 ppu;

    // Flags
    bool pendingNmi = false;

    // Counters
    std::uint64_t clockCounter = 0;

    // Constants
    static constexpr std::uint16_t CPU_RAM_START = 0x0000;
    static constexpr std::uint16_t CPU_RAM_END = 0x1FFF;
    static constexpr std::uint16_t CPU_PPU_START = 0x2000;
    static constexpr std::uint16_t CPU_PPU_END = 0x3FFF;
    static constexpr std::uint16_t CPU_APU_START = 0x4000;
    static constexpr std::uint16_t CPU_APU_END = 0x4017;
    static constexpr std::uint16_t CPU_CARTRIDGE_START = 0x8000;
    static constexpr std::uint16_t CPU_CARTRIDGE_END = 0xFFFF;
    static constexpr std::uint32_t MAXIMUM_ROM_SIZE = 0xFFFF + 1;

    static constexpr std::uint16_t PPU_PATTERN_TABLE_START = 0x0000;
    static constexpr std::uint16_t PPU_PATTERN_TABLE_END = 0x1FFF;
    static constexpr std::uint16_t PPU_NAME_TABLE_START = 0x2000;
    static constexpr std::uint16_t PPU_NAME_TABLE_END = 0x2FFF;
    static constexpr std::uint16_t PPU_PALETTE_START = 0x3F00;
    static constexpr std::uint16_t PPU_PALETTE_END = 0x3FFF;
};

}  // namespace NES

#endif  //NES_EMULATOR_NES_HPP
