#ifndef NES_EMULATOR_PALETTERAM_HPP
#define NES_EMULATOR_PALETTERAM_HPP

#include "common/Image.hpp"
#include "memory/Ram.hpp"

namespace NES {

/**
 * @brief Palette RAM of 32 bytes
 *
 * This class inherits the implementation from Ram (so it applies mirroring by default to
 * all address that exceeds the size of the RAM). Also, it applies the background mirroring
 * to the addresses that are in the range 0x3F10 - 0x3F1F.
 *
 * @see PaletteRam::bgMirror
 */
class PaletteRam : private Ram<0x20> {
  public:
    /**
     * @brief Default constructor
     */
    PaletteRam() = default;

    /**
     * @brief Default destructor
     */
    ~PaletteRam() = default;

  public:  // Memory operations
    /**
     * @brief Write to palette RAM.
     *
     * Applies background mirroring if necessary
     *
     * @param addr Address to write to
     * @param data Data to write
     */
    constexpr void write(std::uint16_t addr, std::uint8_t data) noexcept { Ram::write(bgMirror(addr), data); }

    /**
     * @brief Read from palette RAM.
     *
     * Applies background mirroring if necessary
     *
     * @param addr Address to read from
     * @return Data at addr
     */
    [[nodiscard]] constexpr std::uint8_t read(std::uint16_t addr) const noexcept { return Ram::read(bgMirror(addr)); }

  private:  // Helper methods for memory access
    /**
     * @brief Applies background mirroring to the address
     *
     * @param addr Address to mirror
     * @return Mirrored address
     */
    [[nodiscard]] constexpr std::uint16_t bgMirror(std::uint16_t addr) const noexcept {
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
        // If addr has bit 5 set and is multiple of 4, then write to addr - 0x10
        if ((addr & 0x0013) == 0x0010)
            return addr & ~0x0010;
        return addr;
    }

  public:  // Palette access, abstract user from the internal representation
    /**
     * @brief Get color from palette
     *
     * It abstracts the palette reading and color selection
     *
     * @param palette Index of palette
     * @param color Index of color in palette
     * @return Color
     */
    [[nodiscard]] constexpr Color getColor(std::uint8_t palette, std::uint8_t color) const noexcept {
        constexpr uint8_t PALETTE_SIZE = 4;
        const uint8_t colorValue = PaletteRam::read(static_cast<uint16_t>((palette * PALETTE_SIZE) + (color & (PALETTE_SIZE - 1))));
        return NTSC_PALETTE[colorValue & (NTSC_PALETTE.size() - 1)];
    }

    /**
     * @brief Get a color palette
     *
     * @param palette Index of palette
     * @return Palette
     */
    [[nodiscard]] constexpr std::array<Color, 4> getPalette(std::uint8_t palette) const noexcept {
        return {getColor(palette, 0x00), getColor(palette, 0x01), getColor(palette, 0x02), getColor(palette, 0x03)};
    }

    // Array that contains the color mapping of each palette value
    static constexpr std::array<Color, 0x40> NTSC_PALETTE = {{
        {84, 84, 84},  // 0x00
        {0, 30, 116},  // 0x01
        {8, 16, 144},  // 0x02
        {48, 0, 136},  // 0x03
        {68, 0, 100},  // 0x04
        {92, 0, 48},   // 0x05
        {84, 4, 0},    // 0x06
        {60, 24, 0},   // 0x07
        {32, 42, 0},   // 0x08
        {8, 58, 0},    // 0x09
        {0, 64, 0},    // 0x0A
        {0, 60, 0},    // 0x0B
        {0, 50, 60},   // 0x0C
        {0, 0, 0},     // 0x0D
        {0, 0, 0},     // 0x0E
        {0, 0, 0},     // 0x0F

        {152, 150, 152},  // 0x10
        {8, 76, 196},     // 0x11
        {48, 50, 236},    // 0x12
        {92, 30, 228},    // 0x13
        {136, 20, 176},   // 0x14
        {160, 20, 100},   // 0x15
        {152, 34, 32},    // 0x16
        {120, 60, 0},     // 0x17
        {84, 90, 0},      // 0x18
        {40, 114, 0},     // 0x19
        {8, 124, 0},      // 0x1A
        {0, 118, 40},     // 0x1B
        {0, 102, 120},    // 0x1C
        {0, 0, 0},        // 0x1D
        {0, 0, 0},        // 0x1E
        {0, 0, 0},        // 0x1F

        {236, 238, 236},  // 0x20
        {76, 154, 236},   // 0x21
        {120, 124, 236},  // 0x22
        {176, 98, 236},   // 0x23
        {228, 84, 236},   // 0x24
        {236, 88, 180},   // 0x25
        {236, 106, 100},  // 0x26
        {212, 136, 32},   // 0x27
        {160, 170, 0},    // 0x28
        {116, 196, 0},    // 0x29
        {76, 208, 32},    // 0x2A
        {56, 204, 108},   // 0x2B
        {56, 180, 204},   // 0x2C
        {60, 60, 60},     // 0x2D
        {0, 0, 0},        // 0x2E
        {0, 0, 0},        // 0x2F

        {236, 238, 236},  // 0x30
        {168, 204, 236},  // 0x31
        {188, 188, 236},  // 0x32
        {212, 178, 236},  // 0x33
        {236, 174, 236},  // 0x34
        {236, 174, 212},  // 0x35
        {236, 180, 176},  // 0x36
        {228, 196, 144},  // 0x37
        {204, 210, 120},  // 0x38
        {180, 222, 120},  // 0x39
        {168, 226, 144},  // 0x3A
        {152, 226, 180},  // 0x3B
        {160, 214, 228},  // 0x3C
        {160, 162, 160},  // 0x3D
        {0, 0, 0},        // 0x3E
        {0, 0, 0},        // 0x3F
    }};
};

}  // namespace NES

#endif  //NES_EMULATOR_PALETTERAM_HPP
