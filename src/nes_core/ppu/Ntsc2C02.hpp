#ifndef NES_EMULATOR_NTSC2C02_HPP
#define NES_EMULATOR_NTSC2C02_HPP

#include <bitset>
#include "common/Image.hpp"
#include "memory/PaletteRam.hpp"
#include "memory/VRam.hpp"

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
     * @brief Write to the OAM Address Register.
     * @param data The data (address) to write.
     */
    void oamAddrWrite(std::uint8_t data) noexcept;

    /**
     * @brief Write to the OAM memory.
     * @param data The data to write.
     *
     * The data will be written to the OAM address specified by the OAM Address Register.
     */
    void oamDataWrite(std::uint8_t data) noexcept;

    /**
     * @brief Read from the OAM memory.
     * The data will be read from the OAM address specified by the OAM Address Register.
     * @return The data read.
     */
    std::uint8_t oamDataRead() noexcept;

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

    void ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t ppuBusRead(std::uint16_t addr) const noexcept;

  public:
    /**
     * @brief Returns if the PPU is currently rendering the screen.
     * @return True if the PPU is rendering (frame is ready), false otherwise.
     */
    [[nodiscard]] constexpr bool isFrameCompleted() const noexcept { return frameCompleted; }

  public:
    /**
     * @brief Per-pixel background opacity, used to resolve sprite/background priority.
     * True where the background has a non-transparent pixel.
     */
    using BackgroundMask = std::array<bool, 256 * 240>;

    [[nodiscard]] Image<Color, 256, 240> renderFrame() const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderBackground(BackgroundMask* opaque = nullptr) const noexcept;

    void renderForeground(Image<Color, 256, 240>& img, const BackgroundMask* bgOpaque = nullptr) const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderNametable1() const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderNametable2() const noexcept;

    [[nodiscard]] Image<Color, 256, 240> renderForeground() const noexcept;

    [[nodiscard]] Image<Color, 128, 128> renderPatternTable(std::uint8_t table, std::uint8_t palette = 0xFF) const noexcept;

  public:  // Sprite getters
    [[nodiscard]] Image<Palette, 8, 8> getSprite(std::uint8_t tile, bool rightTable) const noexcept;

    [[nodiscard]] Image<Palette, 8, 8> getSprite(std::uint8_t tileI, std::uint8_t tileJ, bool rightTable) const noexcept;

    [[nodiscard]] Color getColor(std::uint8_t palette, std::uint8_t color) const noexcept;

    [[nodiscard]] std::array<Color, 4> getPalette(std::uint8_t palette) const noexcept;

  private:
    /**
     * @brief Counts how many sprites in OAM sit on the given scanline.
     * @param line The scanline to check.
     * @return The number of sprites overlapping that scanline.
     */
    [[nodiscard]] int spritesOnScanline(int line) const noexcept;

    /* Counters */
    int cycles = 0;
    int scanline = 0;
    bool frameCompleted = false;
    int frameCounter = 0;

    /* Helper variables */
    std::uint8_t dataBuffer = 0;

    /* Memory */
    NesSystem& sys;

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

    /** Object Attribute Memory
     * The NES has 256 bytes of OAM, which is used to store the attributes of the sprites. Each
     * sprite uses 4 bytes to store its attributes: Y position, tile index, attributes and X position.
     * The OAM is accessed through the OAMADDR and OAMDATA registers of the PPU. The OAM is also used
     * for the DMA transfer, which allows to copy 256 bytes of memory from the CPU RAM to the OAM in a single operation.
     * @see https://www.nesdev.org/wiki/PPU_OAM
     */
    Ram<256> oam;
    std::uint8_t r_OAMADDR = 0;  // OAMADDR register, to keep track of the current address to read/write OAMDATA

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

    std::bitset<8> r_PPUCTRL;  // PPU Control Register ($2000)

    enum MaskFlags {
        Grayscale = 0,         // Greyscale display  (0: normal color, 1: produce a greyscale display)
        BgLeftmost = 1,        // Background left column enable  (0: hide, 1: show)
        SpLeftmost = 2,        // Sprite left column enable  (0: hide, 1: show)
        RenderBackground = 3,  // Background enable  (0: hide, 1: show)
        RenderSprite = 4,      // Sprite enable  (0: hide, 1: show)
        Red = 5,               // Red emphasis  (0: normal color, 1: emphasized color)
        Green = 6,             // Green emphasis  (0: normal color, 1: emphasized color)
        Blue = 7,              // Blue emphasis  (0: normal color, 1: emphasized color)
    };

    std::bitset<8> r_PPUMASK;  // PPU Mask Register ($2001)

    enum StatusFlags {
        SpriteOverflow = 5,    // Sprite overflow  (0: less than eight, 1: eight or more sprites on current scanline)
        SpriteZero = 6,        // Sprite 0 hit (0: no, 1: nonzero pixel of sprite 0 overlaps a nonzero background pixel)
        VerticalBlanking = 7,  // Vertical blanking  (0: not in vblank; 1: in vblank)
    };

    std::bitset<8> r_PPUSTATUS;  // PPU Status Register ($2002)

    /* The PPU uses two registers to handle scrolling: r_PPUADDR and r_TMPADDR.
     * Both use the same enconding to store the scroll information, defined as follows:
     *
     *     yyy NN YYYYY XXXXX
     *     ||| || ||||| +++++-- coarse X scroll (tile scroll inside nametable)
     *     ||| || +++++-------- coarse Y scroll
     *     ||| ++-------------- nametable select
     *     +++----------------- fine Y scroll (pixel scroll inside tile)
     *
     * It also uses an extra register, r_fineX, to store the fine X scroll.
     * However, in our scenario we are not looking for a cycle-level accurate emulation,
     * so we can use some extra variables to store the scroll information individually.
     *
     * @see Scrolling reference: https://www.nesdev.org/wiki/PPU_scrolling
     */

    std::uint16_t r_PPUADDR;     // PPU Address Register ($2006)
    std::uint16_t r_TMPADDR;     // Temporary VRAM address (15 bits)
    bool r_writeToggle = false;  // First or second write toggle (1 bit)

    std::uint8_t r_coarseX;       // Coarse X scroll (5 bits)
    std::uint8_t r_coarseY;       // Coarse Y scroll (5 bits)
    std::uint8_t r_fineX;         // Fine X scroll (3 bits)
    std::uint8_t r_fineY;         // Fine Y scroll (3 bits)
    std::uint8_t r_nametableIdx;  // Nametable index (2 bits)

    /* Scroll values latched at the end of the visible region, used for rendering.
     * Games with a split screen (e.g. SMB) overwrite the scroll registers during
     * vblank to draw the status bar, so the live values at the end of the frame are
     * not the ones used to render the playfield. */
    std::uint8_t r_renderCoarseX;
    std::uint8_t r_renderCoarseY;
    std::uint8_t r_renderFineX;
    std::uint8_t r_renderFineY;
    std::uint8_t r_renderNametableIdx;

    // Constants
    static constexpr std::uint16_t PPU_PATTERN_TABLE_START = 0x0000;
    static constexpr std::uint16_t PPU_PATTERN_TABLE_END = 0x1FFF;
    static constexpr std::uint16_t PPU_NAME_TABLE_START = 0x2000;
    static constexpr std::uint16_t PPU_NAME_TABLE_END = 0x2FFF;
    static constexpr std::uint16_t PPU_PALETTE_START = 0x3F00;
    static constexpr std::uint16_t PPU_PALETTE_END = 0x3FFF;
};

}  // namespace NES

#endif  //NES_EMULATOR_NTSC2C02_HPP
