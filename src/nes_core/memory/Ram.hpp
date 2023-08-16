#ifndef NES_EMULATOR_RAM_HPP
#define NES_EMULATOR_RAM_HPP

#include <array>
#include <cstdint>

namespace NES {

/**
 * @brief RAM with resizable size and mirroring
 * @tparam size Size of the RAM in bytes
 *
 * This class implements a RAM which support multiple sizes. It also applies mirroring to the
 * addresses that exceed the size of the RAM. The mirroring is applied by ANDing the address
 * with the size of the RAM.
 *
 * @pre RAM size can be addressed using a 16 bit address bus (i.e. size <= 64 KB)
 * @pre Size is a power of 2
 */
template <std::size_t size>
class Ram {
  public:
    /**
     * @brief Default constructor
     */
    Ram() {
        static_assert(size <= 0xFFFF + 1, "Ram size must be less than 64 KB");
        static_assert((size & (size - 1)) == 0, "Ram size must be power of 2");
    };

    /**
     * @brief Default destructor
     */
    ~Ram() = default;

  public:  // Memory operations
    /**
     * @brief Write to RAM
     *
     * Applies mirroring if necessary
     *
     * @param addr Address to write to
     * @param data Data to write
     */
    constexpr void write(std::uint16_t addr, std::uint8_t data) noexcept { memory[mirror(addr)] = data; }

    /**
     * @brief Read from RAM
     *
     * Applies mirroring if necessary
     *
     * @param addr Address to read from
     * @return Data at addr
     */
    [[nodiscard]] constexpr std::uint8_t read(std::uint16_t addr) const noexcept { return memory[mirror(addr)]; }

  private:  // Helper methods for memory access
    /**
     * @brief Applies mirroring to the address
     *
     * Mirroring is applied when the address exceeds the size of the RAM. The mirroring is
     * applied by ANDing and taking the less significant bits of the address (according to
     * the size of the RAM).
     *
     * @param addr Address to mirror
     * @return Mirrored address
     */
    [[nodiscard]] constexpr std::uint16_t mirror(std::uint16_t addr) const noexcept { return addr & (size - 1); }

  protected:
    // RAM memory
    std::array<std::uint8_t, size> memory{};  // Initialized to 0
};

}  // namespace NES

#endif  //NES_EMULATOR_RAM_HPP
