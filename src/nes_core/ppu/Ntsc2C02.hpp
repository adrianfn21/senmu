#ifndef NES_EMULATOR_NTSC2C02_HPP
#define NES_EMULATOR_NTSC2C02_HPP

#include <bitset>
#include "common/Image.hpp"
#include "memory/Ram.hpp"

namespace NES {

class NesSystem;  // Forward declaration

// TODO write documentation

class NTSC2C02 {
  public:
    /**
     * @brief PPU constructor.
     * @param nes The NES system which the PPU belongs to.
     */
    NTSC2C02(NesSystem& nes);

    /**
     * @brief PPU destructor.
     */
    ~NTSC2C02() = default;

    /**
     * @brief Executes a single PPU cycle.
     */
    void cycle();

    /**
     * @brief Resets the PPU.
     */
    void reset() noexcept;

    /**
     * @brief Returns if the PPU is currently rendering the screen.
     * @return True if the PPU is rendering, false otherwise.
     */
    bool isRendering() const noexcept;

    /**
     * @brief Write to the PPU Control Register.
     * @param data The data to write.
     *
     * @see NTS2C02::ControllerFlags
     */
    void controllerWrite(std::uint8_t data) noexcept;

    /**
     * @brief Write to the PPU Mask Register.
     * @param data The data to write.
     *
     * @see NTS2C02::MaskFlags
     */
    void maskWrite(std::uint8_t data) noexcept;

    /**
     * @brief Read the PPU Status Register.
     * @return The status of the PPU.
     *
     * @note Reading the status register will clear the vblank flag, thus modifying the
     *       internal state of the PPU. Also it resets the NTSC2C02::addressWrite latch.
     *
     * @see NTS2C02::StatusFlags
     */
    std::uint8_t statusRead() noexcept;

    /**
     * @brief Write to the PPU Scroll Register.
     * @param data The data to write.
     *
     * @see NTS2C02::ScrollFlags
     */
    void scrollWrite(std::uint8_t data) noexcept;

    /**
     * @brief Write to the PPU Address Register.
     * @param data The data to write.
     *
     * The addresses are written in two calls to this function. The first call writes the
     * high byte of the address, and the second call writes the low byte of the address.
     *
     * @see NTS2C02::AddressFlags
     */
    void addressWrite(std::uint8_t data) noexcept;

    /**
     * @brief Write to the PPU Data Register.
     *
     * The data will be written to the address specified by the address register. The
     * address register will be incremented by 1 or 32 depending on the increment mode
     * specified in the control register.
     *
     * @param data The data to write.
     */
    void dataWrite(std::uint8_t data) noexcept;

    /**
     * @brief Read from the PPU Data Register.
     *
     * The data will be read from the address specified by the address register. The
     * address register will be incremented by 1 or 32 depending on the increment mode
     * specified in the control register.
     *
     * The read is buffered, so the data returned is the data that was read in the
     * previous cycle (except for readings to the palette RAM).
     *
     * @return The data read.
     */
    [[nodiscard]] std::uint8_t dataRead() noexcept;

  public:
    /**
     * @brief Returns if the PPU is currently rendering the screen.
     * @return True if the PPU is rendering (frame is ready), false otherwise.
     */
    [[nodiscard]] constexpr bool isFrameCompleted() const noexcept { return frameCompleted; }

  public:
    [[nodiscard]] Image<Color, 256, 240> renderFrame() const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderBackground() const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderForeground() const noexcept;

    [[nodiscard]] Image<Color, 128, 128> renderPatternTable(uint8_t table, uint8_t palette = 0xFF) const noexcept;

  private:
    /* Counters */
    int cycles = 0;
    int scanline = 0;
    bool frameCompleted = false;

    /* Helper variables */
    bool waitingLowByte = false;
    std::uint8_t dataBuffer = 0;

    /* Memory */
    NesSystem& sys;

    /* Registers */
    enum ControllerFlags {
        NametableIdx1 = 0,    // Nametable lsb index  (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
        NametableIdx2 = 1,    // Nametable msb index
        IncrementMode = 2,    // Increment mode  (0: add 1, going across; 1: add 32, going down)
        PatternTable = 3,     // Sprite pattern table address for 8x8 sprites  (0: $0000; 1: $1000; ignored in 8x16 mode)
        BackgroundTable = 4,  // Background pattern table address  (0: $0000; 1: $1000)
        Size = 5,             // Sprite size  (0: 8x8 pixels; 1: 8x16 pixels)
        Master = 6,           // PPU master/slave select  (0: read backdrop from EXT pins; 1: output color on EXT pins)
        GenerateNMI = 7,      // Generate an NMI at the start of the vertical blanking interval  (0: off; 1: on)
    };

    std::bitset<8> r_PPUCTRL;  // PPU Control Register

    enum MaskFlags {
        Grayscale = 0,   // Greyscale display  (0: normal color, 1: produce a greyscale display)
        BgLeftmost = 1,  // Background left column enable  (0: hide, 1: show)
        SpLeftmost = 2,  // Sprite left column enable  (0: hide, 1: show)
        Background = 3,  // Background enable  (0: hide, 1: show)
        Sprite = 4,      // Sprite enable  (0: hide, 1: show)
        Red = 5,         // Red emphasis  (0: normal color, 1: emphasized color)
        Green = 6,       // Green emphasis  (0: normal color, 1: emphasized color)
        Blue = 7,        // Blue emphasis  (0: normal color, 1: emphasized color)
    };

    std::bitset<8> r_PPUMASK;  // PPU Mask Register

    enum StatusFlags {
        SpriteOverflow = 5,    // Sprite overflow  (0: less than eight, 1: eight or more sprites on current scanline)
        SpriteZero = 6,        // Sprite 0 hit (0: no, 1: nonzero pixel of sprite 0 overlaps a nonzero background pixel)
        VerticalBlanking = 7,  // Vertical blanking  (0: not in vblank; 1: in vblank)
    };

    std::bitset<8> r_PPUSTATUS;  // PPU Status Register

    uint16_t r_PPUSCROLL;  // PPU Scroll Register
    uint16_t r_PPUADDR;    // PPU Address Register
    uint8_t r_PPUDATA;     // PPU Data Register
};

}  // namespace NES

#endif  //NES_EMULATOR_NTSC2C02_HPP
