#ifndef NES_EMULATOR_MOS6502_HPP
#define NES_EMULATOR_MOS6502_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <string>

namespace NES {

/**
 * @brief Class that represents the MOS6502 CPU of the NES.
 *
 * The MOS6502 is an 8-bit microprocessor that was designed in 1975. It was used in many home computers and game consoles.
 * A modified version of the MOS6502 was used in the NES (named 2A03), which also included a sound chip.
 * With this module we try to emulate the behaviour of the MOS6502.
 *
 * This CPU is little-endian and has a 16-bit address bus and 8-bit registers. It has 3 general purpose registers (A, X and Y),
 * a stack pointer (SP) and a program counter (PC). It also has a status register (P) that contains the status of the CPU.
 *
 * The MOS6502 has 56 instructions, each with a different opcode. Also, each instruction can operate using different
 * memory addressing modes, existing 13 different addressing modes. However, all the opcodes are grouped in a 16x16
 * lookup table that pairs each instruction with an addressing mode (not all instructions support all addressing modes).
 */

class NesSystem;  // Forward declaration

class MOS6502 {
  public:
    /**
     * @brief MOS6502 constructor.
     *
     * @param bus The bus that connects the CPU with its memory system.
     */
    explicit MOS6502(NesSystem& nes);

    /**
     * @brief MOS6502 destructor.
     */
    ~MOS6502();

    /**
     * @brief Executes a single CPU cycle.
     */
    void cycle();

    /**
     * @brief Executes a single CPU instruction.
     */
    void step();

    /**
     * @brief Returns the opcode of the current instruction.
     */
    [[nodiscard]] constexpr std::uint8_t getOpcode() const { return opcode; }

    /**
     * @brief Returns the current value of the program counter.
     */
    [[nodiscard]] constexpr std::uint16_t getPC() const { return r_PC; }

    /**
     * @brief Sets the value of the program counter.
     */
    constexpr void setPC(std::uint16_t pc) { r_PC = pc; }

    /**
     * @brief Returns the number of cycles executed.
     */
    [[nodiscard]] constexpr std::uint64_t getCycles() const { return cyclesCounter; }

    /**
     * @brief Returns the number of instructions executed.
     */
    [[nodiscard]] constexpr std::uint64_t getInstructions() const { return instructionsCounter; }

    /**
     * @brief Resets the CPU.
     *
     * This method resets the CPU to its initial state. It sets the registers to 0, clears the status register
     * the program counter to the address stored in 0xFFFC and the stack pointer to 0xFD.
     */
    void reset();

    /**
     * @brief Executes an Interrupt Request.
     *
     * This method executes an Interrupt Request. It pushes the program counter and the status register to the stack,
     * disables interrupts and sets the program counter to the address stored in 0xFFFE.
     */
    void irq();

    /**
     * @brief Executes a Non-Maskable Interrupt.
     *
     * This method executes a Non-Maskable Interrupt (cannot be avoided). It pushes the program counter and the status register
     * to the stack, disables interrupts and sets the program counter to the address stored in 0xFFFA.
     */
    void nmi();

  private:
    /* Connections */
    // CPU connected to NES buses to perform reads and writes
    NesSystem& sys;

    /* Helper variables to simulation */
    std::uint8_t cycles;   // Remaining cycles for the current instruction
    std::uint8_t opcode;   // Current opcode
    std::uint8_t fetched;  // Fetched data
    std::uint16_t addr;    // Address the current instruction points to

    // Hardcoded address/page where the stack starts
    static constexpr std::uint16_t STACK_PAGE = 0x0100;

    /* Statistics */
    std::uint64_t cyclesCounter{};        // Number of cycles executed
    std::uint64_t instructionsCounter{};  // Number of instructions executed

    /* CPU Registers */
    std::uint16_t r_PC{};  // Program Counter
    std::uint8_t r_SP{};   // Stack Pointer
    std::uint8_t r_A{};    // Accumulator
    std::uint8_t r_X{};    // Index Register X
    std::uint8_t r_Y{};    // Index Register Y

    enum StatusFlags {
        C = 0,  // Carry Flag
        Z = 1,  // Zero Flag
        I = 2,  // Interrupt Disable
        D = 3,  // Decimal Mode (unused in NES)
        B = 4,  // Break Command
        U = 5,  // Unused
        V = 6,  // Overflow Flag
        N = 7,  // Negative Flag
    };

    std::bitset<8> r_status{};  // Status Register

    /* Instructions supported by the 6502 CPU
     * Reference: https://www.nesdev.org/obelisk-6502-guide/instructions.html
     *
     * Take into account that some instructions have different addressing modes,
     * each of which has a different opcode.
     *
     * These instructions return the number of extra cycles that they can potentially take.
     * These cycles are only added if also its addressing mode requires some extra cycles.
     * Branch instructions are the exception as they always add its extra cycles.
     */

    // Load/Store Instructions
    std::uint8_t LDA();  // Load Accumulator
    std::uint8_t LDX();  // Load Index X
    std::uint8_t LDY();  // Load Index Y
    std::uint8_t STA();  // Store Accumulator
    std::uint8_t STX();  // Store Index X
    std::uint8_t STY();  // Store Index Y

    // Register Transfers Instructions
    std::uint8_t TAX();  // Transfer Accumulator to Index X
    std::uint8_t TAY();  // Transfer Accumulator to Index Y
    std::uint8_t TXA();  // Transfer Index X to Accumulator
    std::uint8_t TYA();  // Transfer Index Y to Accumulator

    // Stack Operations Instructions
    std::uint8_t TSX();  // Transfer Stack Pointer to Index X
    std::uint8_t TXS();  // Transfer Index X to Stack Pointer
    std::uint8_t PHA();  // Push Accumulator
    std::uint8_t PHP();  // Push Processor Status
    std::uint8_t PLA();  // Pull Accumulator
    std::uint8_t PLP();  // Pull Processor Status

    // Logical Instructions
    std::uint8_t AND();  // Logical AND
    std::uint8_t EOR();  // Exclusive OR
    std::uint8_t ORA();  // Logical Inclusive OR
    std::uint8_t BIT();  // Bit Test

    // Arithmetic Instructions
    std::uint8_t ADC();  // Add with Carry
    std::uint8_t SBC();  // Subtract with Carry
    std::uint8_t CMP();  // Compare Accumulator
    std::uint8_t CPX();  // Compare Index X
    std::uint8_t CPY();  // Compare Index Y

    // Increment/Decrement Instructions
    std::uint8_t INC();  // Increment Memory
    std::uint8_t INX();  // Increment Index X
    std::uint8_t INY();  // Increment Index Y
    std::uint8_t DEC();  // Decrement Memory
    std::uint8_t DEX();  // Decrement Index X
    std::uint8_t DEY();  // Decrement Index Y

    // Shift Instructions
    std::uint8_t ASL();  // Arithmetic Shift Left (Memory)
    std::uint8_t ASA();  // Arithmetic Shift Left (Accumulator)
    std::uint8_t LSR();  // Logical Shift Right (Memory)
    std::uint8_t LSA();  // Logical Shift Right (Accumulator)
    std::uint8_t ROL();  // Rotate Left (Memory)
    std::uint8_t ROA();  // Rotate Left (Accumulator)
    std::uint8_t ROR();  // Rotate Right (Memory)
    std::uint8_t RAA();  // Rotate Right (Accumulator)

    // Jump/Calls Instructions
    std::uint8_t JMP();  // Jump
    std::uint8_t JSR();  // Jump to Subroutine
    std::uint8_t RTS();  // Return from Subroutine

    // Branch Instructions
    std::uint8_t BCC();  // Branch if Carry Clear
    std::uint8_t BCS();  // Branch if Carry Set
    std::uint8_t BNE();  // Branch if Not Equal
    std::uint8_t BEQ();  // Branch if Equal
    std::uint8_t BPL();  // Branch if Positive
    std::uint8_t BMI();  // Branch if Minus
    std::uint8_t BVC();  // Branch if Overflow Clear
    std::uint8_t BVS();  // Branch if Overflow Set

    // Status Flag Instructions
    std::uint8_t CLC();  // Clear Carry Flag
    std::uint8_t CLD();  // Clear Decimal Mode
    std::uint8_t CLI();  // Clear Interrupt Disable
    std::uint8_t CLV();  // Clear Overflow Flag
    std::uint8_t SEC();  // Set Carry Flag
    std::uint8_t SED();  // Set Decimal Flag
    std::uint8_t SEI();  // Set Interrupt Disable

    // System Functions
    std::uint8_t BRK();  // Force Interrupt
    std::uint8_t NOP();  // No Operation
    std::uint8_t RTI();  // Return from Interrupt

    // Unofficial Instructions
    std::uint8_t LAX();  // Load Accumulator and Index X
    std::uint8_t SAX();  // Store Accumulator and Index X
    std::uint8_t DCP();  // Decrement Memory and Compare
    std::uint8_t ISC();  // Increment Memory and Subtract with Carry
    std::uint8_t SLO();  // Shift Left Memory and OR with Accumulator
    std::uint8_t RLA();  // Rotate Left Memory and AND with Accumulator
    std::uint8_t SRE();  // Shift Right Memory and XOR with Accumulator
    std::uint8_t RRA();  // Rotate Right Memory and Add with Carry

    // Not defined behavior
    std::uint8_t XXX();  // Illegal Opcode

    /* Addressing Modes supported by the 6502 CPU
     * Reference: https://www.nesdev.org/obelisk-6502-guide/addressing.html
     *
     * All modes will calculate the target address of the instruction and update the PC accordingly to the number of bytes it requires.
     * The address is stored in the addr variable, so the instruction can fetch the data from the memory using it.
     * Special cases:
     * - IMP: does not fetch any data.
     * - IND: only used by JMP instruction, only fetches the address to jump to (addr).
     *
     * These addressing modes return the number of extra cycles that they can potentially require.
     * They will only affect to the instructions that does not have a fixed number of cycles.
     */
    std::uint8_t IMP();                   // Implicit
    [[maybe_unused]] std::uint8_t ACC();  // Accumulator (unused, behaviour adapted to AAL and RAL instructions)
    std::uint8_t IMM();                   // Immediate
    std::uint8_t ZP0();                   // Zero Page
    std::uint8_t ZPX();                   // X-Indexed Zero Page
    std::uint8_t ZPY();                   // Y-Indexed Zero Page
    std::uint8_t REL();                   // Relative (only used by branch instructions)
    std::uint8_t ABS();                   // Absolute
    std::uint8_t ABX();                   // X-Indexed Absolute
    std::uint8_t ABY();                   // Y-Indexed Absolute
    std::uint8_t IND();                   // Absolute Indirect (only used by JMP)
    std::uint8_t IZX();                   // X-Indexed Zero Page Indirect
    std::uint8_t IZY();                   // Zero Page Indirect Y-Indexed

    /* Instruction Set OpCode Matrix
     * Reference table (page 10): http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
     * Unofficial supported opcodes: https://www.nesdev.org/wiki/Programming_with_unofficial_opcodes
     */
    struct OpcodeInst {
        std::string name;
        std::uint8_t opcode;
        std::uint8_t (MOS6502::*instruction)() = nullptr;
        std::uint8_t (MOS6502::*addrMode)() = nullptr;
        std::uint8_t cycles;
    };

    const std::array<OpcodeInst, 256> opcodeLUT{{
        {"BRK", 0x00, &MOS6502::BRK, &MOS6502::IMP, 7},  //
        {"ORA", 0x01, &MOS6502::ORA, &MOS6502::IZX, 6},  //
        {"XXX", 0x02, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"SLO", 0x03, &MOS6502::SLO, &MOS6502::IZX, 8},  // Unofficial
        {"NOP", 0x04, &MOS6502::NOP, &MOS6502::ZP0, 3},  // Unofficial
        {"ORA", 0x05, &MOS6502::ORA, &MOS6502::ZP0, 3},  //
        {"ASL", 0x06, &MOS6502::ASL, &MOS6502::ZP0, 5},  //
        {"SLO", 0x07, &MOS6502::SLO, &MOS6502::ZP0, 5},  // Unofficial
        {"PHP", 0x08, &MOS6502::PHP, &MOS6502::IMP, 3},  //
        {"ORA", 0x09, &MOS6502::ORA, &MOS6502::IMM, 2},  //
        {"ASL", 0x0A, &MOS6502::ASA, &MOS6502::IMP, 2},  // ASL with Acc => ASA
        {"XXX", 0x0B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"NOP", 0x0C, &MOS6502::NOP, &MOS6502::ABS, 4},  // Unofficial
        {"ORA", 0x0D, &MOS6502::ORA, &MOS6502::ABS, 4},  //
        {"ASL", 0x0E, &MOS6502::ASL, &MOS6502::ABS, 6},  //
        {"SLO", 0x0F, &MOS6502::SLO, &MOS6502::ABS, 6},  // Unofficial

        {"BPL", 0x10, &MOS6502::BPL, &MOS6502::REL, 2},  //
        {"ORA", 0x11, &MOS6502::ORA, &MOS6502::IZY, 5},  //
        {"XXX", 0x12, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"SLO", 0x13, &MOS6502::SLO, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0x14, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"ORA", 0x15, &MOS6502::ORA, &MOS6502::ZPX, 4},  //
        {"ASL", 0x16, &MOS6502::ASL, &MOS6502::ZPX, 6},  //
        {"SLO", 0x17, &MOS6502::SLO, &MOS6502::ZPX, 6},  // Unofficial
        {"CLC", 0x18, &MOS6502::CLC, &MOS6502::IMP, 2},  //
        {"ORA", 0x19, &MOS6502::ORA, &MOS6502::ABY, 4},  //
        {"NOP", 0x1A, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"SLO", 0x1B, &MOS6502::SLO, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0x1C, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"ORA", 0x1D, &MOS6502::ORA, &MOS6502::ABX, 4},  //
        {"ASL", 0x1E, &MOS6502::ASL, &MOS6502::ABX, 7},  //
        {"SLO", 0x1F, &MOS6502::SLO, &MOS6502::ABX, 7},  // Unofficial

        {"JSR", 0x20, &MOS6502::JSR, &MOS6502::ABS, 6},  //
        {"AND", 0x21, &MOS6502::AND, &MOS6502::IZX, 6},  //
        {"XXX", 0x22, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"RLA", 0x23, &MOS6502::RLA, &MOS6502::IZX, 8},  // Unofficial
        {"BIT", 0x24, &MOS6502::BIT, &MOS6502::ZP0, 3},  //
        {"AND", 0x25, &MOS6502::AND, &MOS6502::ZP0, 3},  //
        {"ROL", 0x26, &MOS6502::ROL, &MOS6502::ZP0, 5},  //
        {"RLA", 0x27, &MOS6502::RLA, &MOS6502::ZP0, 5},  // Unofficial
        {"PLP", 0x28, &MOS6502::PLP, &MOS6502::IMP, 4},  //
        {"AND", 0x29, &MOS6502::AND, &MOS6502::IMM, 2},  //
        {"ROL", 0x2A, &MOS6502::ROA, &MOS6502::IMP, 2},  // ROL with Acc => ROA
        {"XXX", 0x2B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"BIT", 0x2C, &MOS6502::BIT, &MOS6502::ABS, 4},  //
        {"AND", 0x2D, &MOS6502::AND, &MOS6502::ABS, 4},  //
        {"ROL", 0x2E, &MOS6502::ROL, &MOS6502::ABS, 6},  //
        {"RLA", 0x2F, &MOS6502::RLA, &MOS6502::ABS, 6},  // Unofficial

        {"BMI", 0x30, &MOS6502::BMI, &MOS6502::REL, 2},  //
        {"AND", 0x31, &MOS6502::AND, &MOS6502::IZY, 5},  //
        {"XXX", 0x32, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"RLA", 0x33, &MOS6502::RLA, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0x34, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"AND", 0x35, &MOS6502::AND, &MOS6502::ZPX, 4},  //
        {"ROL", 0x36, &MOS6502::ROL, &MOS6502::ZPX, 6},  //
        {"RLA", 0x37, &MOS6502::RLA, &MOS6502::ZPX, 6},  // Unofficial
        {"SEC", 0x38, &MOS6502::SEC, &MOS6502::IMP, 2},  //
        {"AND", 0x39, &MOS6502::AND, &MOS6502::ABY, 4},  //
        {"NOP", 0x3A, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"RLA", 0x3B, &MOS6502::RLA, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0x3C, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"AND", 0x3D, &MOS6502::AND, &MOS6502::ABX, 4},  //
        {"ROL", 0x3E, &MOS6502::ROL, &MOS6502::ABX, 7},  //
        {"RLA", 0x3F, &MOS6502::RLA, &MOS6502::ABX, 7},  // Unofficial

        {"RTI", 0x40, &MOS6502::RTI, &MOS6502::IMP, 6},  //
        {"EOR", 0x41, &MOS6502::EOR, &MOS6502::IZX, 6},  //
        {"XXX", 0x42, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"SRE", 0x43, &MOS6502::SRE, &MOS6502::IZX, 8},  // Unofficial
        {"NOP", 0x44, &MOS6502::NOP, &MOS6502::ZP0, 3},  // Unofficial
        {"EOR", 0x45, &MOS6502::EOR, &MOS6502::ZP0, 3},  //
        {"LSR", 0x46, &MOS6502::LSR, &MOS6502::ZP0, 5},  //
        {"SRE", 0x47, &MOS6502::SRE, &MOS6502::ZP0, 5},  // Unofficial
        {"PHA", 0x48, &MOS6502::PHA, &MOS6502::IMP, 3},  //
        {"EOR", 0x49, &MOS6502::EOR, &MOS6502::IMM, 2},  //
        {"LSR", 0x4A, &MOS6502::LSA, &MOS6502::IMP, 2},  // LSR with Acc => LSA
        {"XXX", 0x4B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"JMP", 0x4C, &MOS6502::JMP, &MOS6502::ABS, 3},  //
        {"EOR", 0x4D, &MOS6502::EOR, &MOS6502::ABS, 4},  //
        {"LSR", 0x4E, &MOS6502::LSR, &MOS6502::ABS, 6},  //
        {"SRE", 0x4F, &MOS6502::SRE, &MOS6502::ABS, 6},  // Unofficial

        {"BVC", 0x50, &MOS6502::BVC, &MOS6502::REL, 2},  //
        {"EOR", 0x51, &MOS6502::EOR, &MOS6502::IZY, 5},  //
        {"XXX", 0x52, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"SRE", 0x53, &MOS6502::SRE, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0x54, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"EOR", 0x55, &MOS6502::EOR, &MOS6502::ZPX, 4},  //
        {"LSR", 0x56, &MOS6502::LSR, &MOS6502::ZPX, 6},  //
        {"SRE", 0x57, &MOS6502::SRE, &MOS6502::ZPX, 6},  // Unofficial
        {"CLI", 0x58, &MOS6502::CLI, &MOS6502::IMP, 2},  //
        {"EOR", 0x59, &MOS6502::EOR, &MOS6502::ABY, 4},  //
        {"NOP", 0x5A, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"SRE", 0x5B, &MOS6502::SRE, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0x5C, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"EOR", 0x5D, &MOS6502::EOR, &MOS6502::ABX, 4},  //
        {"LSR", 0x5E, &MOS6502::LSR, &MOS6502::ABX, 7},  //
        {"SRE", 0x5F, &MOS6502::SRE, &MOS6502::ABX, 7},  // Unofficial

        {"RTS", 0x60, &MOS6502::RTS, &MOS6502::IMP, 6},  //
        {"ADC", 0x61, &MOS6502::ADC, &MOS6502::IZX, 6},  //
        {"XXX", 0x62, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"RRA", 0x63, &MOS6502::RRA, &MOS6502::IZX, 8},  // Unofficial
        {"NOP", 0x64, &MOS6502::NOP, &MOS6502::ZP0, 3},  // Unofficial
        {"ADC", 0x65, &MOS6502::ADC, &MOS6502::ZP0, 3},  //
        {"ROR", 0x66, &MOS6502::ROR, &MOS6502::ZP0, 5},  //
        {"RRA", 0x67, &MOS6502::RRA, &MOS6502::ZP0, 5},  // Unofficial
        {"PLA", 0x68, &MOS6502::PLA, &MOS6502::IMP, 4},  //
        {"ADC", 0x69, &MOS6502::ADC, &MOS6502::IMM, 2},  //
        {"ROR", 0x6A, &MOS6502::RAA, &MOS6502::IMP, 2},  //  ROR with Acc => RAA
        {"XXX", 0x6B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"JMP", 0x6C, &MOS6502::JMP, &MOS6502::IND, 5},  //
        {"ADC", 0x6D, &MOS6502::ADC, &MOS6502::ABS, 4},  //
        {"ROR", 0x6E, &MOS6502::ROR, &MOS6502::ABS, 6},  //
        {"RRA", 0x6F, &MOS6502::RRA, &MOS6502::ABS, 6},  // Unofficial

        {"BVS", 0x70, &MOS6502::BVS, &MOS6502::REL, 2},  //
        {"ADC", 0x71, &MOS6502::ADC, &MOS6502::IZY, 5},  //
        {"XXX", 0x72, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"RRA", 0x73, &MOS6502::RRA, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0x74, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"ADC", 0x75, &MOS6502::ADC, &MOS6502::ZPX, 4},  //
        {"ROR", 0x76, &MOS6502::ROR, &MOS6502::ZPX, 6},  //
        {"RRA", 0x77, &MOS6502::RRA, &MOS6502::ZPX, 6},  // Unofficial
        {"SEI", 0x78, &MOS6502::SEI, &MOS6502::IMP, 2},  //
        {"ADC", 0x79, &MOS6502::ADC, &MOS6502::ABY, 4},  //
        {"NOP", 0x7A, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"RRA", 0x7B, &MOS6502::RRA, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0x7C, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"ADC", 0x7D, &MOS6502::ADC, &MOS6502::ABX, 4},  //
        {"ROR", 0x7E, &MOS6502::ROR, &MOS6502::ABX, 7},  //
        {"RRA", 0x7F, &MOS6502::RRA, &MOS6502::ABX, 7},  // Unofficial

        {"NOP", 0x80, &MOS6502::NOP, &MOS6502::IMM, 2},  // Unofficial
        {"STA", 0x81, &MOS6502::STA, &MOS6502::IZX, 6},  //
        {"XXX", 0x82, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"SAX", 0x83, &MOS6502::SAX, &MOS6502::IZX, 6},  // Unofficial
        {"STY", 0x84, &MOS6502::STY, &MOS6502::ZP0, 3},  //
        {"STA", 0x85, &MOS6502::STA, &MOS6502::ZP0, 3},  //
        {"STX", 0x86, &MOS6502::STX, &MOS6502::ZP0, 3},  //
        {"SAX", 0x87, &MOS6502::SAX, &MOS6502::ZP0, 3},  // Unofficial
        {"DEY", 0x88, &MOS6502::DEY, &MOS6502::IMP, 2},  //
        {"XXX", 0x89, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"TXA", 0x8A, &MOS6502::TXA, &MOS6502::IMP, 2},  //
        {"XXX", 0x8B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"STY", 0x8C, &MOS6502::STY, &MOS6502::ABS, 4},  //
        {"STA", 0x8D, &MOS6502::STA, &MOS6502::ABS, 4},  //
        {"STX", 0x8E, &MOS6502::STX, &MOS6502::ABS, 4},  //
        {"SAX", 0x8F, &MOS6502::SAX, &MOS6502::ABS, 4},  // Unofficial

        {"BCC", 0x90, &MOS6502::BCC, &MOS6502::REL, 2},  //
        {"STA", 0x91, &MOS6502::STA, &MOS6502::IZY, 6},  //
        {"XXX", 0x92, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"XXX", 0x93, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"STY", 0x94, &MOS6502::STY, &MOS6502::ZPX, 4},  //
        {"STA", 0x95, &MOS6502::STA, &MOS6502::ZPX, 4},  //
        {"STX", 0x96, &MOS6502::STX, &MOS6502::ZPY, 4},  //
        {"SAX", 0x97, &MOS6502::SAX, &MOS6502::ZPY, 4},  // Unofficial
        {"TYA", 0x98, &MOS6502::TYA, &MOS6502::IMP, 2},  //
        {"STA", 0x99, &MOS6502::STA, &MOS6502::ABY, 5},  //
        {"TXS", 0x9A, &MOS6502::TXS, &MOS6502::IMP, 2},  //
        {"XXX", 0x9B, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"XXX", 0x9C, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"STA", 0x9D, &MOS6502::STA, &MOS6502::ABX, 5},  //
        {"XXX", 0x9E, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"XXX", 0x9F, &MOS6502::XXX, &MOS6502::IMP, 0},  //

        {"LDY", 0xA0, &MOS6502::LDY, &MOS6502::IMM, 2},  //
        {"LDA", 0xA1, &MOS6502::LDA, &MOS6502::IZX, 6},  //
        {"LDX", 0xA2, &MOS6502::LDX, &MOS6502::IMM, 2},  //
        {"LAX", 0xA3, &MOS6502::LAX, &MOS6502::IZX, 6},  // Unofficial
        {"LDY", 0xA4, &MOS6502::LDY, &MOS6502::ZP0, 3},  //
        {"LDA", 0xA5, &MOS6502::LDA, &MOS6502::ZP0, 3},  //
        {"LDX", 0xA6, &MOS6502::LDX, &MOS6502::ZP0, 3},  //
        {"LAX", 0xA7, &MOS6502::LAX, &MOS6502::ZP0, 3},  // Unofficial
        {"TAY", 0xA8, &MOS6502::TAY, &MOS6502::IMP, 2},  //
        {"LDA", 0xA9, &MOS6502::LDA, &MOS6502::IMM, 2},  //
        {"TAX", 0xAA, &MOS6502::TAX, &MOS6502::IMP, 2},  //
        {"LAX", 0xAB, &MOS6502::LAX, &MOS6502::IMM, 2},  // Unofficial
        {"LDY", 0xAC, &MOS6502::LDY, &MOS6502::ABS, 4},  //
        {"LDA", 0xAD, &MOS6502::LDA, &MOS6502::ABS, 4},  //
        {"LDX", 0xAE, &MOS6502::LDX, &MOS6502::ABS, 4},  //
        {"LAX", 0xAF, &MOS6502::LAX, &MOS6502::ABS, 4},  // Unofficial

        {"BCS", 0xB0, &MOS6502::BCS, &MOS6502::REL, 2},  //
        {"LDA", 0xB1, &MOS6502::LDA, &MOS6502::IZY, 5},  //
        {"XXX", 0xB2, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"LAX", 0xB3, &MOS6502::LAX, &MOS6502::IZY, 5},  // Unofficial
        {"LDY", 0xB4, &MOS6502::LDY, &MOS6502::ZPX, 4},  //
        {"LDA", 0xB5, &MOS6502::LDA, &MOS6502::ZPX, 4},  //
        {"LDX", 0xB6, &MOS6502::LDX, &MOS6502::ZPY, 4},  //
        {"LAX", 0xB7, &MOS6502::LAX, &MOS6502::ZPY, 4},  // Unofficial
        {"CLV", 0xB8, &MOS6502::CLV, &MOS6502::IMP, 2},  //
        {"LDA", 0xB9, &MOS6502::LDA, &MOS6502::ABY, 4},  //
        {"TSX", 0xBA, &MOS6502::TSX, &MOS6502::IMP, 2},  //
        {"XXX", 0xBB, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"LDY", 0xBC, &MOS6502::LDY, &MOS6502::ABX, 4},  //
        {"LDA", 0xBD, &MOS6502::LDA, &MOS6502::ABX, 4},  //
        {"LDX", 0xBE, &MOS6502::LDX, &MOS6502::ABY, 4},  //
        {"LAX", 0xBF, &MOS6502::LAX, &MOS6502::ABY, 4},  // Unofficial

        {"CPY", 0xC0, &MOS6502::CPY, &MOS6502::IMM, 2},  //
        {"CMP", 0xC1, &MOS6502::CMP, &MOS6502::IZX, 6},  //
        {"XXX", 0xC2, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"DCP", 0xC3, &MOS6502::DCP, &MOS6502::IZX, 8},  // Unofficial
        {"CPY", 0xC4, &MOS6502::CPY, &MOS6502::ZP0, 3},  //
        {"CMP", 0xC5, &MOS6502::CMP, &MOS6502::ZP0, 3},  //
        {"DEC", 0xC6, &MOS6502::DEC, &MOS6502::ZP0, 5},  //
        {"DCP", 0xC7, &MOS6502::DCP, &MOS6502::ZP0, 5},  // Unofficial
        {"INY", 0xC8, &MOS6502::INY, &MOS6502::IMP, 2},  //
        {"CMP", 0xC9, &MOS6502::CMP, &MOS6502::IMM, 2},  //
        {"DEX", 0xCA, &MOS6502::DEX, &MOS6502::IMP, 2},  //
        {"XXX", 0xCB, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"CPY", 0xCC, &MOS6502::CPY, &MOS6502::ABS, 4},  //
        {"CMP", 0xCD, &MOS6502::CMP, &MOS6502::ABS, 4},  //
        {"DEC", 0xCE, &MOS6502::DEC, &MOS6502::ABS, 6},  //
        {"DCP", 0xCF, &MOS6502::DCP, &MOS6502::ABS, 6},  // Unofficial

        {"BNE", 0xD0, &MOS6502::BNE, &MOS6502::REL, 2},  //
        {"CMP", 0xD1, &MOS6502::CMP, &MOS6502::IZY, 5},  //
        {"XXX", 0xD2, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"DCP", 0xD3, &MOS6502::DCP, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0xD4, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"CMP", 0xD5, &MOS6502::CMP, &MOS6502::ZPX, 4},  //
        {"DEC", 0xD6, &MOS6502::DEC, &MOS6502::ZPX, 6},  //
        {"DCP", 0xD7, &MOS6502::DCP, &MOS6502::ZPX, 6},  // Unofficial
        {"CLD", 0xD8, &MOS6502::CLD, &MOS6502::IMP, 2},  //
        {"CMP", 0xD9, &MOS6502::CMP, &MOS6502::ABY, 4},  //
        {"NOP", 0xDA, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"DCP", 0xDB, &MOS6502::DCP, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0xDC, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"CMP", 0xDD, &MOS6502::CMP, &MOS6502::ABX, 4},  //
        {"DEC", 0xDE, &MOS6502::DEC, &MOS6502::ABX, 7},  //
        {"DCP", 0xDF, &MOS6502::DCP, &MOS6502::ABX, 7},  // Unofficial

        {"CPX", 0xE0, &MOS6502::CPX, &MOS6502::IMM, 2},  //
        {"SBC", 0xE1, &MOS6502::SBC, &MOS6502::IZX, 6},  //
        {"XXX", 0xE2, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"ISC", 0xE3, &MOS6502::ISC, &MOS6502::IZX, 8},  // Unofficial
        {"CPX", 0xE4, &MOS6502::CPX, &MOS6502::ZP0, 3},  //
        {"SBC", 0xE5, &MOS6502::SBC, &MOS6502::ZP0, 3},  //
        {"INC", 0xE6, &MOS6502::INC, &MOS6502::ZP0, 5},  //
        {"ISC", 0xE7, &MOS6502::ISC, &MOS6502::ZP0, 5},  // Unofficial
        {"INX", 0xE8, &MOS6502::INX, &MOS6502::IMP, 2},  //
        {"SBC", 0xE9, &MOS6502::SBC, &MOS6502::IMM, 2},  //
        {"NOP", 0xEA, &MOS6502::NOP, &MOS6502::IMP, 2},  //
        {"SBC", 0xEB, &MOS6502::SBC, &MOS6502::IMM, 2},  // Unofficial
        {"CPX", 0xEC, &MOS6502::CPX, &MOS6502::ABS, 4},  //
        {"SBC", 0xED, &MOS6502::SBC, &MOS6502::ABS, 4},  //
        {"INC", 0xEE, &MOS6502::INC, &MOS6502::ABS, 6},  //
        {"ISC", 0xEF, &MOS6502::ISC, &MOS6502::ABS, 6},  // Unofficial

        {"BEQ", 0xF0, &MOS6502::BEQ, &MOS6502::REL, 2},  //
        {"SBC", 0xF1, &MOS6502::SBC, &MOS6502::IZY, 5},  //
        {"XXX", 0xF2, &MOS6502::XXX, &MOS6502::IMP, 0},  //
        {"ISC", 0xF3, &MOS6502::ISC, &MOS6502::IZY, 8},  // Unofficial
        {"NOP", 0xF4, &MOS6502::NOP, &MOS6502::ZPX, 4},  // Unofficial
        {"SBC", 0xF5, &MOS6502::SBC, &MOS6502::ZPX, 4},  //
        {"INC", 0xF6, &MOS6502::INC, &MOS6502::ZPX, 6},  //
        {"ISC", 0xF7, &MOS6502::ISC, &MOS6502::ZPX, 6},  // Unofficial
        {"SED", 0xF8, &MOS6502::SED, &MOS6502::IMP, 2},  //
        {"SBC", 0xF9, &MOS6502::SBC, &MOS6502::ABY, 4},  //
        {"NOP", 0xFA, &MOS6502::NOP, &MOS6502::IMP, 2},  // Unofficial
        {"ISC", 0xFB, &MOS6502::ISC, &MOS6502::ABY, 7},  // Unofficial
        {"NOP", 0xFC, &MOS6502::NOP, &MOS6502::ABX, 4},  // Unofficial
        {"SBC", 0xFD, &MOS6502::SBC, &MOS6502::ABX, 4},  //
        {"INC", 0xFE, &MOS6502::INC, &MOS6502::ABX, 7},  //
        {"ISC", 0xFF, &MOS6502::ISC, &MOS6502::ABX, 7},  // Unofficial
    }};
};

}  // namespace NES

#endif  //NES_EMULATOR_MOS6502_HPP
