#include "Nes.hpp"
#include "cpu/Mos6502.hpp"

/**
 * Main module reference: https://www.nesdev.org/obelisk-6502-guide/addressing.html
 * Detailed reference: https://www.pagetable.com/c64ref/6502/?tab=3
 */

namespace NES {

/* Implicit.
 * For many 6502 instructions the source and destination of the information to be manipulated is implied directly by the function
 * of the instruction itself and no further operand needs to be specified. Operations like 'Clear Carry Flag' (CLC) and 'Return from
 * Subroutine' (RTS) are implicit.
 */
std::uint8_t MOS6502::IMP() {
    // Doesn't need to fetch data
    return 0;
}

/* Accumulator.
 * Some instructions have an option to operate directly upon the accumulator.
 * The programmer specifies this by using a special operand value, 'A'.
 */
std::uint8_t MOS6502::ACC() {
    fetched = r_A;
    return 0;
}

/* Immediate.
 * Immediate addressing allows the programmer to directly specify an 8 bit constant within the instruction.
 * It is indicated by a '#' symbol followed by a numeric expression.
 */
std::uint8_t MOS6502::IMM() {
    fetched = sys.cpuBusRead(r_PC++);
    return 0;
}

/* Zero Page.
 * An instruction using zero page addressing mode has only an 8-bit address operand.
 * This limits it to addressing only the first 256 bytes of memory (e.g. $0000 to $00FF) where the most significant byte of the address is always zero.
 * In zero-page mode only the least significant byte of the address is held in the instruction making it shorter by one byte (important for space-saving)
 * and one less memory fetch during execution (important for speed).
 */
std::uint8_t MOS6502::ZP0() {
    addr = sys.cpuBusRead(r_PC++);
    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* X-Indexed Zero Page.
 * The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8-bit zero-page address
 * from the instruction and adding the current value of the X register to it.
 * For example if the X register contains $0F and the instruction LDA $80,X is executed then the accumulator
 * will be loaded from $008F (e.g. $80 + $0F => $8F).
 *
 * The address calculation wraps around if the sum of the base address and the register exceed $FF.
 * If we repeat the last example but with $FF in the X register then the accumulator will be loaded from $007F (e.g. $80 + $FF => $7F) and not $017F.
 */
std::uint8_t MOS6502::ZPX() {
    addr = static_cast<std::uint8_t>(sys.cpuBusRead(r_PC++) + r_X);  // Yes, it does not handle overflow
    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* Y-Indexed Zero Page.
 * The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8-bit zero-page address
 * from the instruction and adding the current value of the Y register to it.
 * This mode can only be used with the LDX and STX instructions.
 */
std::uint8_t MOS6502::ZPY() {
    addr = static_cast<std::uint8_t>(sys.cpuBusRead(r_PC++) + r_Y);  // Yes, it does not handle overflow
    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* Relative.
 * Relative addressing mode is used by branch instructions (e.g. BEQ, BNE, etc.) which contain a signed 8-bit relative offset (e.g. -128 to +127)
 * which is added to program counter if the condition is true.
 * As the program counter itself is incremented during instruction execution by two the effective address range for the target instruction
 * must be with -126 to +129 bytes of the branch.
 */
std::uint8_t MOS6502::REL() {
    // This value has to be used as a signed value later
    fetched = sys.cpuBusRead(r_PC++);

    // Implementation trick: relative addressing is only used by branch instructions, which can potentially take +1 or +2 extra cycles.
    // Also, in cycle() function we only take the extra cycle if both instruction and addressing mode require it (i.e., cyclesInst & cyclesAddr).
    // So, we can set the return cycles of this function to 0b0011 in order to always match that +1 or +2 extra cycles.
    return 0x03;
}

/* Absolute.
 * Instructions using absolute addressing contain a full 16-bit address to identify the target location.
 */
std::uint8_t MOS6502::ABS() {
    std::uint16_t low = sys.cpuBusRead(r_PC++);
    std::uint16_t high = sys.cpuBusRead(r_PC++);
    addr = static_cast<std::uint16_t>((high << 8) | low);
    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* X-Indexed Absolute.
 * The address to be accessed by an instruction using X register indexed absolute addressing is computed by taking the 16-bit address
 * from the instruction and added the contents of the X register.
 * For example if X contains $92 then an STA $2000,X instruction will store the accumulator at $2092 (e.g. $2000 + $92).
 */
std::uint8_t MOS6502::ABX() {
    std::uint16_t low = sys.cpuBusRead(r_PC++);
    std::uint16_t high = sys.cpuBusRead(r_PC++);
    auto baseAddr = static_cast<std::uint16_t>((high << 8) | low);
    addr = baseAddr + r_X;
    fetched = sys.cpuBusRead(addr);

    // If page boundary is crossed, then is needed to add an "oops cycle" in order to ADD the high byte of the address
    return (baseAddr & 0xFF00) != (addr & 0xFF00) ? 1 : 0;
}

/* Y-Indexed Absolute.
 * The Y register indexed absolute addressing mode is the same as the previous mode only with the contents of the Y register
 * added to the 16-bit address from the instruction.
 */
std::uint8_t MOS6502::ABY() {
    std::uint16_t low = sys.cpuBusRead(r_PC++);
    std::uint16_t high = sys.cpuBusRead(r_PC++);
    auto baseAddr = static_cast<std::uint16_t>((high << 8) | low);
    addr = baseAddr + r_Y;
    fetched = sys.cpuBusRead(addr);

    // If page boundary is crossed, then is needed to add an "oops cycle" in order to ADD the high byte of the address
    return (baseAddr & 0xFF00) != (addr & 0xFF00) ? 1 : 0;
}

/* Absolute Indirect.
 * JMP is the only 6502 instruction to support indirection. The instruction contains a 16-bit address which identifies the location of
 * the least significant byte of another 16-bit memory address which is the real target of the instruction.
 * For example if location $0120 contains $FC and location $0121 contains $BA then the instruction JMP ($0120) will cause the next instruction
 * execution to occur at $BAFC (e.g. the contents of $0120 and $0121).
 */
std::uint8_t MOS6502::IND() {
    std::uint16_t ptrLow = sys.cpuBusRead(r_PC++);
    std::uint16_t ptrHigh = sys.cpuBusRead(r_PC++);
    auto ptr = static_cast<std::uint16_t>((ptrHigh << 8) | ptrLow);

    // Simulate hardware bug (https://www.nesdev.org/6502bugs.txt)
    // An indirect JMP (xxFF) will fail because the MSB will be fetched from
    // address xx00 instead of page xx+1.
    if (ptrLow == 0x00FF) {  // Page boundary hardware bug (add lower byte of the address without carry)
        addr = static_cast<std::uint16_t>((sys.cpuBusRead(ptr & 0xFF00) << 8) | sys.cpuBusRead(ptr));
    } else {  // Normal operation
        addr = static_cast<std::uint16_t>((sys.cpuBusRead(ptr + 1) << 8) | sys.cpuBusRead(ptr + 0));
    }

    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* X-Indexed Zero Page Indirect.
 * Indexed indirect addressing is normally used in conjunction with a table of address held on zero page. The address of the table is
 * taken from the instruction and the X register added to it (with zero page wrap around) to give the location of the least significant
 * byte of the target address.
 */
std::uint8_t MOS6502::IZX() {
    std::uint16_t indirect = sys.cpuBusRead(r_PC++);
    std::uint16_t low = sys.cpuBusRead((indirect + r_X) & 0x00FF);
    std::uint16_t high = sys.cpuBusRead((indirect + r_X + 1) & 0x00FF);
    addr = static_cast<std::uint16_t>((high << 8) | low);
    fetched = sys.cpuBusRead(addr);
    return 0;
}

/* Zero Page Indirect Y-Indexed.
 * Indexed indirect addressing is the most common indirection mode used on the 6502. In instruction contains the zero-page location
 * of the least significant byte of 16-bit address. The Y register is dynamically added to this value to generate the actual target
 * address for operation.
 */
std::uint8_t MOS6502::IZY() {
    std::uint16_t indirect = sys.cpuBusRead(r_PC++);
    std::uint16_t low = sys.cpuBusRead(indirect);
    std::uint16_t high = sys.cpuBusRead((indirect + 1) & 0x00FF);
    auto baseAddr = static_cast<std::uint16_t>((high << 8) | low);
    addr = baseAddr + r_Y;
    fetched = sys.cpuBusRead(addr);

    // If page boundary is crossed, then is needed to add an "oops cycle" in order to ADD the high byte of the address
    return (baseAddr & 0xFF00) != (addr & 0xFF00) ? 1 : 0;
}

}  // namespace NES