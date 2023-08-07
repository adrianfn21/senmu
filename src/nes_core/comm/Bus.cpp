#include "Bus.hpp"

namespace NES {

void Bus::addDevice(BusSubscriber* device) {
    devices.push_back(device);
}

void Bus::write(std::uint16_t addr, std::uint8_t data) const noexcept {
    findDevice(addr)->writeHandler(addr, data);
}

std::uint8_t Bus::read(std::uint16_t addr) const noexcept {
    return findDevice(addr)->readHandler(addr);
}

BusSubscriber* Bus::findDevice(std::uint16_t addr) const noexcept {
    auto device = std::find_if(devices.begin(), devices.end(), [addr](const BusSubscriber* device) {
        return addr >= device->startAddr && addr <= device->endAddr;  //
    });
    assert(device != devices.end() && "No device found for address");
    return *device;
}

}  // namespace NES