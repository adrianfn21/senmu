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
enum Button { RIGHT = 0x80, LEFT = 0x40, DOWN = 0x20, UP = 0x10, START = 0x08, SELECT = 0x04, B = 0x02, A = 0x01 };

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
     * @brief Updates the controller state, reading from the button buffer
     *
     * @param port Controller port
     */
    constexpr void write(Controller port) noexcept { input[port] = buttonBuffer[port]; }

    /**
     * @brief Reads the next bit from the controller
     *
     * @param port Controller port
     * @return The next bit from the controller
     */
    [[nodiscard]] constexpr std::uint8_t read(Controller port) noexcept {
        std::uint8_t data = input[port] & 0x01;
        input[port] = static_cast<std::uint8_t>(input[port] >> 1);
        return data;
    }

    /**
     * @brief Sets the button state of the controller, buffering it for the next read from controller operation
     * @param port Controller port
     * @param button Button to set
     * @param pressed Button state (true if pressed, false if released)
     */
    constexpr void setButton(Controller port, Button button, bool pressed) noexcept {
        if (pressed) {
            buttonBuffer[port] |= static_cast<std::uint8_t>(button);
        } else {
            buttonBuffer[port] &= ~static_cast<std::uint8_t>(button);
        }
    }

  public:  // Wrapper functions when accessing to $4016 and $4017 addresses
    constexpr void write(std::uint8_t port) noexcept { write(static_cast<Controller>(port)); }

    [[nodiscard]] constexpr std::uint8_t read(std::uint8_t port) noexcept { return read(static_cast<Controller>(port)); }

  private:
    // Store the shift register for each controller
    std::array<std::uint8_t, 2> input{};

    // Buffers the buttons pressed while the CPU is not reading from the controller
    std::array<std::uint8_t, 2> buttonBuffer{};
};

}  // namespace NES

#endif  //NES_EMULATOR_NESCONTROLLER_HPP
