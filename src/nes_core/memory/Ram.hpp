#ifndef NES_EMULATOR_RAM_HPP
#define NES_EMULATOR_RAM_HPP

#include <array>
#include <cstdint>

namespace NES {

// TODO add documentation

template <std::size_t size>
class Ram {
  public:
    Ram() {
        static_assert(size <= 0xFFFF + 1, "Ram size must be less than 64 KB");
        static_assert((size & (size - 1)) == 0, "Ram size must be power of 2");
    };

    ~Ram() = default;

    constexpr void write(std::uint16_t addr, std::uint8_t data) noexcept { memory[addr & (size - 1)] = data; }

    [[nodiscard]] constexpr std::uint8_t read(std::uint16_t addr) const noexcept { return memory[addr & (size - 1)]; }

  private:
    std::array<std::uint8_t, size> memory{};  // Initialized to 0
};

}  // namespace NES

#endif  //NES_EMULATOR_RAM_HPP
