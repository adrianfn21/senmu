#ifndef NES_EMULATOR_RAM_HPP
#define NES_EMULATOR_RAM_HPP

#include <cstdint>
#include <vector>
#include "comm/Bus.hpp"

namespace NES {

class Ram : private BusSubscriber {
  public:
    Ram(Bus& bus, std::uint16_t startAddr, std::uint16_t endAddr);

    ~Ram() override = default;

    void writeHandler(std::uint16_t addr, std::uint8_t data) override;

    [[nodiscard]] std::uint8_t readHandler(std::uint16_t addr) override;

  private:
    std::vector<std::uint8_t> memory;
};

}  // namespace NES

#endif  //NES_EMULATOR_RAM_HPP
