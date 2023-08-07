#ifndef NES_EMULATOR_NES_HPP
#define NES_EMULATOR_NES_HPP

#include <cstdint>
#include "comm/Bus.hpp"
#include "cpu/Mos6502.hpp"
#include "memory/Ram.hpp"

namespace NES {

// TODO: pending to add documentation

class NesSystem {
  public:
    NesSystem();
    ~NesSystem();

    void loadRom(const std::vector<std::uint8_t>& rom);

    void reset() noexcept;
    void cycle() noexcept;
    void step() noexcept;

    bool isRunning();

    void setPC(std::uint16_t pc) noexcept;
    [[nodiscard]] std::uint16_t getPC() const noexcept;

    [[nodiscard]] uint64_t getCycles() const noexcept;
    [[nodiscard]] uint64_t getInstructions() const noexcept;

  public:
    void write(std::uint16_t addr, std::uint8_t data) const noexcept;
    [[nodiscard]] std::uint8_t read(std::uint16_t addr) const noexcept;

  private:
    NES::Bus cpuBus;
    NES::Ram ram;
    NES::MOS6502 cpu;

    // Constants
    static constexpr std::uint16_t RAM_SIZE = 0x0800;
    static constexpr std::uint16_t RAM_START = 0x0000;
    static constexpr std::uint16_t RAM_END = RAM_START + RAM_SIZE - 1;
    static constexpr std::uint16_t ROM_SIZE = 0x8000;
    static constexpr std::uint16_t ROM_START = 0x8000;
    static constexpr std::uint16_t ROM_END = ROM_START + ROM_SIZE - 1;
};

}  // namespace NES

#endif  //NES_EMULATOR_NES_HPP
