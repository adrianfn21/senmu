#ifndef NES_EMULATOR_NES_HPP
#define NES_EMULATOR_NES_HPP

#include <cstdint>
#include <vector>
#include "cartridge/GamePak.hpp"
#include "common/Image.hpp"
#include "cpu/Mos6502.hpp"
#include "input/NesController.hpp"
#include "memory/Ram.hpp"
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

    bool isRunning() const;

    void setPC(std::uint16_t pc) noexcept;
    [[nodiscard]] std::uint16_t getPC() const noexcept;

    [[nodiscard]] std::uint64_t getCycles() const noexcept;
    [[nodiscard]] std::uint64_t getInstructions() const noexcept;
    
  public:  // input
    void setButton(Controller controllerPort, Button button, bool pressed) noexcept;

  public:
    void cpuBusWrite(std::uint16_t addr, std::uint8_t data) noexcept;
    [[nodiscard]] std::uint8_t cpuBusRead(std::uint16_t addr) noexcept;

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

    GamePak gpak;
    MOS6502 cpu;
    NTSC2C02 ppu;

    NesController input;

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
    static constexpr std::uint16_t CPU_DMA_OAM = 0x4014;
    static constexpr std::uint16_t CPU_CONTROLLER1 = 0x4016;
    static constexpr std::uint16_t CPU_CONTROLLER2 = 0x4017;
    static constexpr std::uint16_t CPU_CARTRIDGE_START = 0x8000;
    static constexpr std::uint16_t CPU_CARTRIDGE_END = 0xFFFF;
};

}  // namespace NES

#endif  //NES_EMULATOR_NES_HPP
