#ifndef NES_EMULATOR_NESCONTROLLER_HPP
#define NES_EMULATOR_NESCONTROLLER_HPP

#include "memory/Ram.hpp"

namespace NES {

/**
 * @brief Input buttons for the NES controller
 *
 * The NES controller has 8 buttons, which are represented by the Input enum.
 * Each button is represented by a bit in the controller shift register.
 *
 * @see https://www.nesdev.org/wiki/Standard_controller
 */
enum Button { A = 0x80, B = 0x40, SELECT = 0x20, START = 0x10, UP = 0x08, DOWN = 0x04, LEFT = 0x02, RIGHT = 0x01 };

/**
 * @brief Controller ports (player 1 and player 2)
 */
enum Controller { P1 = 0x00, P2 = 0x01 };

/**
 * @brief NES controller
 *
 * This module manages the input from the NES standard controller, bringing support for
 * two controllers (player 1 and player 2).
 *
 * Each possible button is represented by a bit in the controller shift register.
 * Also, another bit is used to indicate if the controller port is being read (requested by the CPU).
 * Every time the CPU reads from the controller port, the next bit of the shift register is returned.
 *
 * @see https://wiki.nesdev.com/w/index.php/Controller_port_registers
 */
class NesController {
  public:
    NesController() = default;
    ~NesController() = default;

  public:  // Memory operations
    /**
     * @brief Sets the reading state of the controller
     *
     * @param port Controller port
     * @param reading Reading state (true if reading from controller, false otherwise)
     */
    constexpr void write(Controller port, bool reading) noexcept { readingState[port] = reading; }

    /**
     * @brief Reads the next bit from the controller
     *
     * @param port Controller port
     * @return The next bit from the controller
     */
    [[nodiscard]] constexpr std::uint8_t read(Controller port) noexcept {
        std::uint8_t data = (input[port] & 0x80) > 0;
        input[port] <<= 1;
        return data;
    }

    /**
     * @brief Sets the button state of the controller if it is not being read
     * @param port Controller port
     * @param button Button to set
     * @param pressed Button state (true if pressed, false if released)
     */
    constexpr void setButton(Controller port, Button button, bool pressed) noexcept {
        if (readingState[port])
            return;

        if (pressed) {
            input[port] |= static_cast<std::uint8_t>(button);
        } else {
            input[port] &= ~static_cast<std::uint8_t>(button);
        }
    }

    /**
     * @brief Checks if the controller is currently being read
     *
     * @param port Controller port
     * @return true if the controller is being read, false otherwise
     */
    [[nodiscard]] constexpr bool isReading(Controller port) const noexcept { return readingState[port]; }

  public:  // Wrapper functions when accessing to $4016 and $4017 addresses
    constexpr void write(std::uint16_t port, bool reading) noexcept { write(static_cast<Controller>(port), reading); }

    [[nodiscard]] constexpr std::uint8_t read(std::uint16_t port) noexcept { return read(static_cast<Controller>(port)); }

  private:
    // Store the shift register for each controller
    std::array<std::uint8_t, 2> input{};

    // Store the reading state for each controller
    std::array<bool, 2> readingState{};
};

}  // namespace NES

#endif  //NES_EMULATOR_NESCONTROLLER_HPP
