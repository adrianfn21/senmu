#include <iostream>
#include "Mos6502.hpp"

/**
 * Main module reference: https://www.nesdev.org/obelisk-6502-guide/reference.html
 * Detailed reference (with illegal opcodes): https://www.pagetable.com/c64ref/6502/?tab=2
 */

namespace NES {

/* Load Accumulator.
 * Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.
 */
uint8_t MOS6502::LDA() {
    r_A = fetched;

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 1;
}

/* Load Index X.
 * Loads a byte of memory into the X register setting the zero and negative flags as appropriate.
 */
uint8_t MOS6502::LDX() {
    r_X = fetched;

    r_status[Z] = r_X == 0x00;
    r_status[N] = r_X & (1 << 7);
    return 1;
}

/* Load Index Y.
 * Loads a byte of memory into the Y register setting the zero and negative flags as appropriate.
 */
uint8_t MOS6502::LDY() {
    r_Y = fetched;

    r_status[Z] = r_Y == 0x00;
    r_status[N] = r_Y & (1 << 7);
    return 1;
}

/* Store Accumulator.
 * Stores the contents of the X register into memory.
 */
uint8_t MOS6502::STA() {
    bus.write(addr, r_A);
    return 0;
}

/* Store Index X.
 * Stores the contents of the X register into memory.
 */
uint8_t MOS6502::STX() {
    bus.write(addr, r_X);
    return 0;
}

/* Store Index Y.
 * Stores the contents of the Y register into memory.
 */
uint8_t MOS6502::STY() {
    bus.write(addr, r_Y);
    return 0;
}

/* Transfer Accumulator to Index X.
 * Copies the current contents of the accumulator into the X register and sets the zero and negative flags as appropriate.
 */
uint8_t MOS6502::TAX() {
    r_X = r_A;

    r_status[Z] = r_X == 0x00;
    r_status[N] = r_X & (1 << 7);
    return 0;
}

/* Transfer Accumulator to Index Y.
 * Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as appropriate.
 */
uint8_t MOS6502::TAY() {
    r_Y = r_A;

    r_status[Z] = r_Y == 0x00;
    r_status[N] = r_Y & (1 << 7);
    return 0;
}

/* Transfer Index X to Accumulator.
 * Copies the current contents of the X register into the accumulator and sets the zero and negative flags as appropriate.
 */
uint8_t MOS6502::TXA() {
    r_A = r_X;

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Transfer Index Y to Accumulator.
 * Copies the current contents of the Y register into the accumulator and sets the zero and negative flags as appropriate.
 */
uint8_t MOS6502::TYA() {
    r_A = r_Y;

    r_status[Z] = r_Y == 0x00;
    r_status[N] = r_Y & (1 << 7);
    return 0;
}

/* Transfer Stack Pointer to Index X.
 * Copies the current contents of the stack register into the X register and sets the zero and negative flags as appropriate.
 */
uint8_t MOS6502::TSX() {
    r_X = r_SP;

    r_status[Z] = r_X == 0x00;
    r_status[N] = r_X & (1 << 7);
    return 0;
}

/* Transfer Index X to Stack Pointer.
 * Copies the current contents of the X register into the stack register.
 */
uint8_t MOS6502::TXS() {
    r_SP = r_X;
    return 0;
}

/* Push Accumulator.
 * Pushes a copy of the accumulator on to the stack.
 */
uint8_t MOS6502::PHA() {
    bus.write(STACK_PAGE + (r_SP--), r_A);
    return 0;
}

/* Push Processor Status.
 * Pushes a copy of the status flags on to the stack.
 */
uint8_t MOS6502::PHP() {
    // Simulate hardware bug (https://www.nesdev.org/6502bugs.txt)
    // The status bits pushed on the stack by PHP have the breakpoint bit set.
    std::bitset<8> statusCopy = r_status;
    statusCopy[B] = true;
    statusCopy[U] = true;
    bus.write(STACK_PAGE + (r_SP--), static_cast<uint8_t>(statusCopy.to_ulong()));
    return 0;
}

/* Pull Accumulator.
 * Pulls an 8 bit value from the stack and into the accumulator. The zero and negative flags are set as appropriate.
 */
uint8_t MOS6502::PLA() {
    r_A = bus.read(STACK_PAGE + (++r_SP));

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Pull Processor Status.
 * Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.
 */
uint8_t MOS6502::PLP() {
    r_status = bus.read(STACK_PAGE + (++r_SP));
    return 0;
}

/* Logical AND.
 * A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
 *
 * Operation: A, Z, N <- A & M
 */
uint8_t MOS6502::AND() {
    r_A = r_A & fetched;

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 1;
}

/* Exclusive OR.
 * An exclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
 *
 * Operation: A, Z, N <- A ^ M
 */
uint8_t MOS6502::EOR() {
    r_A = r_A ^ fetched;

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 1;
}

/* Logical Inclusive OR.
 * An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
 *
 * Operation: A, Z, N <- A | M
 */
uint8_t MOS6502::ORA() {
    r_A = r_A | fetched;

    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 1;
}

/* Bit Test.
 * This instruction is used to test if one or more bits are set in a target memory location. The mask pattern in A is ANDed
 * with the value in memory to set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the value from memory
 * are copied into the N and V flags.
 *
 * Operation: Z, V, N <- A & M, M7, M6
 */
uint8_t MOS6502::BIT() {
    r_status[Z] = (r_A & fetched) == 0x00;
    r_status[V] = fetched & (1 << 6);
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Add with Carry.
 * This instruction adds the contents of a memory location to the accumulator together with the carry bit.
 * If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.
 *
 * Note that NES does not support decimal mode, so the D flag is ignored.
 *
 * Operation: A, C, Z, V, N <- A + M + C
 *
 * Reference: https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
 */
uint8_t MOS6502::ADC() {
    auto sum = static_cast<uint16_t>(r_A + fetched + r_status[C]);

    // Overflow happens if sign of r_A and fetched is equal but sign of sum is different.
    r_status[V] = ((r_A ^ sum) & (fetched ^ sum)) & (1 << 7);  // signed overflow

    r_status[C] = sum > 0xFF;  // unsigned overflow
    r_status[Z] = static_cast<uint8_t>(sum) == 0x00;
    r_status[N] = sum & (1 << 7);

    r_A = static_cast<uint8_t>(sum);
    return 1;
}

/* Subtract with Carry.
 * This instruction subtracts the contents of a memory location to the accumulator together with the negation of
 * the carry bit. If overflow occurs the carry bit is clear, this enables multiple byte subtraction to be performed.
 *
 * Operation: A, C, Z, V, N <- A - M - (1 - C)
 */
uint8_t MOS6502::SBC() {
    // We can simply reuse the ADC function by inverting the value to be added.
    // SBC performs A - M - (1 - C), which is equivalent to: A - M - 1 + C
    // If we invert M, we obtain -M - 1 due to the two's complement representation.
    // So, we end up obtaining: A - (-M - 1) - 1 + C  =  A + M + C
    fetched = ~fetched;
    return ADC();
}

/* Compare Accumulator.
 * This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.
 *
 * Operation: C, Z, N <- A - M
 */
uint8_t MOS6502::CMP() {
    r_status[C] = r_A >= fetched;
    r_status[Z] = r_A == fetched;
    r_status[N] = (r_A - fetched) & (1 << 7);
    return 1;
}

/* Compare Index X.
 * This instruction compares the contents of the X register with another memory held value and sets the zero and carry flags as appropriate.
 *
 * Operation: C, Z, N <- X - M
 */
uint8_t MOS6502::CPX() {
    r_status[C] = r_X >= fetched;
    r_status[Z] = r_X == fetched;
    r_status[N] = (r_X - fetched) & (1 << 7);
    return 0;
}

/* Compare Index Y.
 * This instruction compares the contents of the Y register with another memory held value and sets the zero and carry flags as appropriate.
 *
 * Operation: C, Z, N <- Y - M
 */
uint8_t MOS6502::CPY() {
    r_status[C] = r_Y >= fetched;
    r_status[Z] = r_Y == fetched;
    r_status[N] = (r_Y - fetched) & (1 << 7);
    return 0;
}

/* Increment Memory.
 * Adds one to the value held at a specified memory location setting the zero and negative flags as appropriate.
 *
 * Operation: M, Z, N <- M + 1
 */
uint8_t MOS6502::INC() {
    fetched++;
    bus.write(addr, fetched);

    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Increment Index X.
 * Adds one to the X register setting the zero and negative flags as appropriate.
 *
 * Operation: X, Z, N <- X + 1
 */
uint8_t MOS6502::INX() {
    r_X++;

    r_status[Z] = r_X == 0x00;
    r_status[N] = r_X & (1 << 7);
    return 0;
}

/* Increment Index Y.
 * Adds one to the Y register setting the zero and negative flags as appropriate.
 *
 * Operation: Y, Z, N <- Y + 1
 */
uint8_t MOS6502::INY() {
    r_Y++;

    r_status[Z] = r_Y == 0x00;
    r_status[N] = r_Y & (1 << 7);
    return 0;
}

/* Decrement Memory.
 * Subtracts one from the value held at a specified memory location setting the zero and negative flags as appropriate.
 *
 * Operation: M, Z, N <- M - 1
 */
uint8_t MOS6502::DEC() {
    fetched--;
    bus.write(addr, fetched);

    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Decrement Index X.
 * Subtracts one from the X register setting the zero and negative flags as appropriate.
 *
 * Operation: X, Z, N <- X - 1
 */
uint8_t MOS6502::DEX() {
    r_X--;

    r_status[Z] = r_X == 0x00;
    r_status[N] = r_X & (1 << 7);
    return 0;
}

/* Decrement Index Y.
 * Subtracts one from the Y register setting the zero and negative flags as appropriate.
 *
 * Operation: Y, Z, N <- Y - 1
 */
uint8_t MOS6502::DEY() {
    r_Y--;

    r_status[Z] = r_Y == 0x00;
    r_status[N] = r_Y & (1 << 7);
    return 0;
}

/* Arithmetic Shift Left (Memory).
 * This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the carry flag.
 * The effect of this operation is to multiply the memory contents by 2 (ignoring 2's complement considerations), setting the carry if the result
 * will not fit in 8 bits.
 *
 * Operation: C, Z, N <- M * 2
 */
uint8_t MOS6502::ASL() {
    // Save old bit 7 in the carry flag
    bool oldC = fetched & (1 << 7);

    fetched = static_cast<uint8_t>(fetched << 1);
    bus.write(addr, fetched);

    r_status[C] = oldC;
    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Arithmetic Shift Left (Accumulator).
 * Same as ASL, but operating with the Accumulator.
 * Operation: C, Z, N <- A * 2
 */
uint8_t MOS6502::ASA() {
    // Save old bit 7 in the carry flag
    bool oldC = r_A & (1 << 7);

    r_A = static_cast<uint8_t>(r_A << 1);

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Logical Shift Right (Memory).
 * Each of the bits in A or M is shift one place to the right. The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.
 *
 * Operation: C, Z, N <- M / 2
 */
uint8_t MOS6502::LSR() {
    // Save old bit 0 in the carry flag
    bool oldC = fetched & (1 << 0);

    fetched = static_cast<uint8_t>(fetched >> 1);
    bus.write(addr, fetched);

    r_status[C] = oldC;
    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Logical Shift Right (Accumulator).
 * Same as LSR, but operating with the Accumulator.
 * Operation: C, Z, N <- A / 2
 */
uint8_t MOS6502::LSA() {
    // Save old bit 0 in the carry flag
    bool oldC = r_A & (1 << 0);

    r_A = static_cast<uint8_t>(r_A >> 1);

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Rotate Left (Memory).
 * Move each of the bits in either A or M one place to the left. Bit 0 is filled with the current value of the carry flag whilst the old bit 7
 * becomes the new carry flag value.
 *
 * Operation: C, Z, N <- M * 2 + C
 */
uint8_t MOS6502::ROL() {
    // Save old bit 7 in the carry flag
    bool oldC = fetched & (1 << 7);

    fetched = static_cast<uint8_t>(fetched << 1);
    fetched |= r_status[C];
    bus.write(addr, fetched);

    r_status[C] = oldC;
    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Rotate Left (Accumulator).
 * Same as ROL, but operating with the Accumulator.
 * Operation: C, Z, N <- A * 2 + C
 */
uint8_t MOS6502::ROA() {
    // Save old bit 7 in the carry flag
    bool oldC = r_A & (1 << 7);

    r_A = static_cast<uint8_t>(r_A << 1);
    r_A |= r_status[C];

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Rotate Right (Memory).
 * Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0
 * becomes the new carry flag value.
 *
 * Operation: C, Z, N <- M / 2 + C * 128
 */
uint8_t MOS6502::ROR() {
    // Save old bit 0 in the carry flag
    bool oldC = fetched & (1 << 0);

    fetched = static_cast<uint8_t>(fetched >> 1);
    fetched |= static_cast<uint8_t>(r_status[C] << 7);
    bus.write(addr, fetched);

    r_status[C] = oldC;
    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 0;
}

/* Rotate Right (Accumulator).
 * Same as ROR, but operating with the Accumulator.
 * Operation: C, Z, N <- A / 2 + C * 128
 */
uint8_t MOS6502::RAA() {
    // Save old bit 0 in the carry flag
    bool oldC = r_A & (1 << 0);

    r_A = static_cast<uint8_t>(r_A >> 1);
    r_A |= static_cast<uint8_t>(r_status[C] << 7);

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Jump.
 * Sets the program counter to the address specified by the operand.
 */
uint8_t MOS6502::JMP() {
    r_PC = addr;
    return 0;
}

/* Jump to Subroutine.
 * The JSR instruction pushes the address (minus one) of the return point on to the stack
 * and then sets the program counter to the target memory address.
 *
 * TODO: can we omit this hardware bug? Apply also to RTS just in case
 */
uint8_t MOS6502::JSR() {
    // Simulate hardware bug (https://www.nesdev.org/6502bugs.txt)
    // Return address pushed on the stack by JSR is one less than actual next
    // instruction.  RTS increments PC after popping.  RTI doesn't.
    uint16_t ret = r_PC - 1;
    bus.write(STACK_PAGE + (r_SP--), static_cast<uint8_t>((ret >> 8) & 0x00FF));
    bus.write(STACK_PAGE + (r_SP--), static_cast<uint8_t>(ret & 0x00FF));

    r_PC = addr;
    return 0;
}

/* Return from Subroutine.
 * The RTS instruction is used at the end of a subroutine to return to the calling routine.
 * It pulls the program counter (minus one) from the stack.
 */
uint8_t MOS6502::RTS() {
    // Simulate hardware bug (https://www.nesdev.org/6502bugs.txt)
    // Return address pushed on the stack by JSR is one less than actual next
    // instruction.  RTS increments PC after popping.  RTI doesn't.
    r_PC = static_cast<uint16_t>(bus.read(STACK_PAGE + (++r_SP)));
    r_PC |= static_cast<uint16_t>(bus.read(STACK_PAGE + (++r_SP)) << 8);
    r_PC++;
    return 0;
}

/* Branch if Carry Clear.
 * If the carry flag is clear then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BCC() {
    if (!r_status[C]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Carry Set.
 * If the carry flag is set then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BCS() {
    if (r_status[C]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Not Equal.
 * If the zero flag is clear then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BNE() {
    if (!r_status[Z]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Equal.
 * If the zero flag is set then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BEQ() {
    if (r_status[Z]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Positive.
 * If the negative flag is clear then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BPL() {
    if (!r_status[N]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Minus.
 * If the negative flag is set then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BMI() {
    if (r_status[N]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Overflow Clear.
 * If the overflow flag is clear then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BVC() {
    if (!r_status[V]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Branch if Overflow Set.
 * If the overflow flag is set then add the relative displacement to the program counter
 * to cause a branch to a new location.
 */
uint8_t MOS6502::BVS() {
    if (r_status[V]) {
        uint16_t oldPC = r_PC;
        r_PC = static_cast<uint16_t>(r_PC + static_cast<int8_t>(fetched));

        // +1 if branch succeeds, +2 if to a new page
        return 1 + static_cast<uint8_t>((r_PC & 0xFF00) != (oldPC & 0xFF00));
    }

    return 0;
}

/* Clear Carry Flag.
 * Set the carry flag to zero.
 */
uint8_t MOS6502::CLC() {
    r_status[C] = false;
    return 0;
}

/* Clear Decimal Mode.
 * Sets the decimal mode flag to zero.
 */
uint8_t MOS6502::CLD() {
    r_status[D] = false;
    return 0;
}

/* Clear Interrupt Disable.
 * Clears the interrupt disable flag allowing normal interrupt requests to be serviced.
 */
uint8_t MOS6502::CLI() {
    r_status[I] = false;
    return 0;
}

/* Clear Overflow Flag.
 * Clears the overflow flag.
 */
uint8_t MOS6502::CLV() {
    r_status[V] = false;
    return 0;
}

/* Set Carry Flag.
 * Set the carry flag to one.
 */
uint8_t MOS6502::SEC() {
    r_status[C] = true;
    return 0;
}

/* Set Decimal Flag.
 * Set the decimal mode flag to one.
 */
uint8_t MOS6502::SED() {
    r_status[D] = true;
    return 0;
}

/* Set Interrupt Disable.
 * Set the interrupt disable flag to one.
 */
uint8_t MOS6502::SEI() {
    r_status[I] = true;
    return 0;
}

/* Force Interrupt.
 * The BRK instruction forces the generation of an interrupt request.
 * The program counter and processor status are pushed on the stack then the IRQ interrupt vector at $FFFE/F
 * is loaded into the PC and the break flag in the status set to one.
 */
uint8_t MOS6502::BRK() {
    r_status[I] = false;
    irq();
    return 0;
}

/* No Operation.
 * Does not do anything.
 */
uint8_t MOS6502::NOP() {
    // Addressing mode in illegal instructions can provoke a page boundary crossing
    return 1;
}

/* Return from Interrupt.
 * The RTI instruction is used at the end of an interrupt processing routine.
 * It pulls the processor flags from the stack followed by the program counter.
 */
uint8_t MOS6502::RTI() {
    r_status = bus.read(STACK_PAGE + (++r_SP));
    r_status[B] = false;
    r_status[U] = false;

    r_PC = static_cast<uint16_t>(bus.read(STACK_PAGE + (++r_SP)));
    r_PC |= static_cast<uint16_t>(bus.read(STACK_PAGE + (++r_SP)) << 8);
    return 0;
}

/* Load Accumulator and Index X.
 * The undocumented LAX instruction loads the accumulator and the index register X from memory.
 */
uint8_t MOS6502::LAX() {
    r_A = fetched;
    r_X = fetched;

    r_status[Z] = fetched == 0x00;
    r_status[N] = fetched & (1 << 7);
    return 1;
}

/* Store Accumulator and Index X.
 * The undocumented SAX instruction performs a bit-by-bit AND operation of the value of the accumulator and the value of the
 * index register X and stores the result in memory.
 *
 * Operation: M <- A & X
 */
uint8_t MOS6502::SAX() {
    bus.write(addr, r_A & r_X);
    return 0;
}

/* Decrement Memory and Compare.
 * This undocumented instruction subtracts 1, in two's complement, from the contents of the addressed memory location.
 * It then subtracts the contents of memory from the contents of the accumulator.
 *
 * Operation: M <- M - 1, C <- A - M
 */
uint8_t MOS6502::DCP() {
    fetched--;
    bus.write(addr, fetched);

    r_status[C] = r_A >= fetched;
    r_status[Z] = r_A == fetched;
    r_status[N] = (r_A - fetched) & (1 << 7);
    return 0;
}

/* Increment Memory and Subtract with Carry.
 * This undocumented instruction adds one to the contents of the addressed memory location.
 * It then subtracts the contents of memory from the contents of the accumulator plus the negation of the carry bit.
 *
 * Operation: M <- M + 1, A <- A - M - (1 - C)
 */
uint8_t MOS6502::ISC() {
    fetched++;
    bus.write(addr, fetched);

    // ISC does not introduce extra cycles
    SBC();
    return 0;
}

/* Shift Left Memory and OR with Accumulator.
 * The undocumented SLO instruction shifts the address memory location 1 bit to the left, with the bit 0 always being set to 0 and
 * the bit 7 output always being contained in the carry flag. It then performs a bit-by-bit "OR" operation on the result and the accumulator
 * and stores the result in the accumulator.
 *
 * Operation: M <- M << 1, A <- A | M
 */
uint8_t MOS6502::SLO() {
    // Save old bit 7 in the carry flag
    bool oldC = fetched & (1 << 7);

    fetched = static_cast<uint8_t>(fetched << 1);
    bus.write(addr, fetched);
    r_A |= fetched;

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Rotate Left Memory and AND with Accumulator.
 * The undocumented RLA instruction shifts the addressed memory left 1 bit, with the input carry being stored in bit 0 and with the input bit 7
 * being stored in the carry flags. It then performs a bit-by-bit AND operation of the result and the value of the accumulator and stores the
 * result back in the accumulator.
 *
 * Operation: M <- M * 2 + C, A <- A & M
 */
uint8_t MOS6502::RLA() {
    // Save old bit 7 in the carry flag
    bool oldC = fetched & (1 << 7);

    fetched = static_cast<uint8_t>(fetched << 1);
    fetched |= r_status[C];
    bus.write(addr, fetched);
    r_A &= fetched;

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Shift Right Memory and XOR with Accumulator.
 * The undocumented SRE instruction shifts the specified memory location 1 bit to the right, with the higher bit of the result always being set to 0,
 * and the low bit which is shifted out of the field being stored in the carry flag. It then performs a bit-by-bit "EXCLUSIVE OR" of the result and
 * the value of the accumulator and stores the result in the accumulator.
 *
 * Operation: M <- M / 2, A <- A ^ M
 */
uint8_t MOS6502::SRE() {
    // Save old bit 0 in the carry flag
    bool oldC = fetched & (1 << 0);

    fetched = static_cast<uint8_t>(fetched >> 1);
    bus.write(addr, fetched);
    r_A ^= fetched;

    r_status[C] = oldC;
    r_status[Z] = r_A == 0x00;
    r_status[N] = r_A & (1 << 7);
    return 0;
}

/* Rotate Right Memory and Add with Carry.
 * The undocumented RRA instruction shifts the addressed memory right 1 bit with bit 0 shifted into the carry and carry shifted into bit 7.
 * It then adds the result and generated carry to the value of the accumulator and stores the result in the accumulator.
 *
 * Operation: M <- M / 2 + C * 128, A <- A + M + C
 */
uint8_t MOS6502::RRA() {
    // Save old bit 0 in the carry flag
    bool oldC = fetched & (1 << 0);

    fetched = static_cast<uint8_t>(fetched >> 1);
    fetched |= static_cast<uint8_t>((r_status[C] << 7));
    bus.write(addr, fetched);

    auto sum = static_cast<uint16_t>(r_A + fetched + oldC);

    // Overflow happens if sign of r_A and fetched is equal but sign of sum is different.
    r_status[V] = ((r_A ^ sum) & (fetched ^ sum)) & (1 << 7);  // signed overflow

    r_status[C] = sum > 0xFF;  // unsigned overflow
    r_status[Z] = static_cast<uint8_t>(sum) == 0x00;
    r_status[N] = sum & (1 << 7);

    r_A = static_cast<uint8_t>(sum);
    return 0;
}

/* Illegal Opcode.
 * Not defined behavior. Stop the emulator.
 */
uint8_t MOS6502::XXX() {
    std::cerr << "Illegal Opcode (0x" << std::hex << static_cast<int>(opcode) << ") at $pc = 0x" << std::hex << static_cast<int>(r_PC) << std::endl;
    exit(1);
}

}  // namespace NES