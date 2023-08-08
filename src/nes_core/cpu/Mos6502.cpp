#include "cpu/Mos6502.hpp"
#include "Nes.hpp"

namespace NES {

MOS6502::MOS6502(NesSystem& nes) : sys(nes), cycles(0), opcode(0xFF), fetched(0), addr(0) {
    reset();
    cycles = 0;
}

MOS6502::~MOS6502() = default;

void MOS6502::cycle() {
    if (cycles == 0) {
        instructionsCounter++;

        opcode = sys.cpuBusRead(r_PC++);
        const OpcodeInst& inst = opcodeLUT[opcode];

        cycles = inst.cycles;
        std::uint8_t addrReqCycles = (this->*inst.addrMode)();
        std::uint8_t instReqCycles = (this->*inst.instruction)();

        // Both cases must require extra cycles, if not then the cost is already implicit in the OpcodeInst
        cycles += addrReqCycles & instReqCycles;
    }

    cycles--;
    cyclesCounter++;
}

void MOS6502::step() {
    while (cycles > 0) {
        cycle();
    }

    cycle();
}

void MOS6502::reset() {
    r_status = 0x00;
    r_A = 0x00;
    r_X = 0x00;
    r_Y = 0x00;

    // Technically, the PC and the status are pushed to the stack, but they are not actually used later
    r_SP = 0xFD;

    // Call to RESET interrupt handler
    const std::uint16_t resetHandlerAddr = 0xFFFC;
    r_PC = static_cast<std::uint16_t>((sys.cpuBusRead(resetHandlerAddr + 1) << 8) | sys.cpuBusRead(resetHandlerAddr));

    cycles = 7;
}

void MOS6502::irq() {
    // This interrupt is only processed if Interrupts are enabled
    if (!r_status[I]) {
        // Pushes the PC to the stack
        sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>((r_PC >> 8) & 0x00FF));
        sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>(r_PC & 0x00FF));

        // Pushes the status register to the stack
        r_status[B] = false;
        r_status[U] = true;
        r_status[I] = true;
        sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>(r_status.to_ulong()));

        // Call to the interrupt handler for IRQ
        constexpr std::uint16_t irqHandlerAddr = 0xFFFE;
        r_PC = static_cast<std::uint16_t>((sys.cpuBusRead(irqHandlerAddr + 1) << 8) | sys.cpuBusRead(irqHandlerAddr));

        cycles = 7;
    }
}

void MOS6502::nmi() {
    // Pushes the PC to the stack
    sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>((r_PC >> 8) & 0x00FF));
    sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>(r_PC & 0x00FF));

    // Pushes the status register to the stack
    r_status[B] = false;
    r_status[U] = true;
    r_status[I] = true;
    sys.cpuBusWrite(STACK_PAGE + (r_SP--), static_cast<std::uint8_t>(r_status.to_ulong()));

    // Call to the interrupt handler for IRQ
    constexpr std::uint16_t nmiHandlerAddr = 0xFFFA;
    r_PC = static_cast<std::uint16_t>((sys.cpuBusRead(nmiHandlerAddr + 1) << 8) | sys.cpuBusRead(nmiHandlerAddr));

    cycles = 8;
}

}  // namespace NES