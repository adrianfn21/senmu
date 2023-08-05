#ifndef NES_EMULATOR_RAM_HPP
#define NES_EMULATOR_RAM_HPP

#include <array>
#include <cstdint>
#include "comm/Bus.hpp"

namespace NES {

class Ram : private BusSubscriber {
  public:
    Ram(Bus& bus, uint16_t startAddr, uint16_t endAddr);

    ~Ram() override = default;

    void writeHandler(uint16_t addr, uint8_t data) override;

    [[nodiscard]] uint8_t readHandler(uint16_t addr) override;

  private:
    std::array<uint8_t, 2 * 1024> memory{};  // 2KB
};

}  // namespace NES

#endif  //NES_EMULATOR_RAM_HPP
