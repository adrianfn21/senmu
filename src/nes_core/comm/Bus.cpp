#include "Bus.hpp"

namespace NES {

void Bus::addDevice(BusSubscriber* device) {
    devices.push_back(device);
}

void Bus::write(uint16_t addr, uint8_t data) const noexcept {
    findDevice(addr)->writeHandler(addr, data);
}

uint8_t Bus::read(uint16_t addr) const noexcept {
    return findDevice(addr)->readHandler(addr);
}

BusSubscriber* Bus::findDevice(uint16_t addr) const noexcept {
    auto device = std::find_if(devices.begin(), devices.end(), [addr](const BusSubscriber* device) {
        return addr >= device->startAddr && addr <= device->endAddr;  //
    });
    assert(device != devices.end() && "No device found for address");
    return *device;
}

}  // namespace NES