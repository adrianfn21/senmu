#ifndef NES_EMULATOR_NES_HPP
#define NES_EMULATOR_NES_HPP

#include <cstdint>
#include <vector>
#include "cartridge/GamePak.hpp"
#include "cpu/Mos6502.hpp"
#include "memory/Ram.hpp"

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
    void step() noexcept;

    bool isRunning();

    void setPC(std::uint16_t pc) noexcept;
    [[nodiscard]] std::uint16_t getPC() const noexcept;

    [[nodiscard]] uint64_t getCycles() const noexcept;
    [[nodiscard]] uint64_t getInstructions() const noexcept;

  public:
    void cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t cpuBusRead(std::uint16_t addr) const noexcept;

    void ppuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t ppuBusRead(std::uint16_t addr) const noexcept;

  private:
    Ram<2 * 1024> ram;  // 2 KB
    GamePak gpak;
    MOS6502 cpu;

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
