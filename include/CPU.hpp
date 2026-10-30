#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <vector>
#include <string>

class Bus;

class CPU {
public:
    CPU();
    ~CPU();

    void connectBus(Bus* bus) { m_bus = bus; }

    // Registers
    uint8_t a = 0x00;        // Accumulator
    uint8_t x = 0x00;        // X Register
    uint8_t y = 0x00;        // Y Register
    uint8_t stp = 0xFD;      // Stack Pointer
    uint16_t pc = 0x0000;    // Program Counter
    uint8_t status = 0x24;   // Status Register

    // Flags
    enum Flags6502 {
        C = (1 << 0), // Carry Bit
        Z = (1 << 1), // Zero
        I = (1 << 2), // Disable Interrupts
        D = (1 << 3), // Decimal Mode (unused in 2A03)
        B = (1 << 4), // Break
        U = (1 << 5), // Unused / Expansion
        V = (1 << 6), // Overflow
        N = (1 << 7), // Negative
    };

    void clock();
    void reset();
    void irq();
    void triggerNMI() { m_nmiPending = true; }

    bool complete() const { return m_cycles == 0; }

private:
    Bus* m_bus = nullptr;
    bool m_nmiPending = false;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    uint8_t getFlag(Flags6502 f) const { return (status & f) ? 1 : 0; }
    void setFlag(Flags6502 f, bool v) {
        if (v) status |= f;
        else status &= ~f;
    }

    uint8_t m_fetched = 0x00;
    uint16_t m_addrAbs = 0x0000;
    uint16_t m_addrRel = 0x0000;
    uint8_t m_opcode = 0x00;
    uint8_t m_cycles = 0;
    uint32_t m_clockCount = 0;

    struct Instruction {
        std::string name;
        uint8_t (CPU::*operate)(void) = nullptr;
        uint8_t (CPU::*addrmode)(void) = nullptr;
        uint8_t cycles = 0;
    };

    std::vector<Instruction> m_lookup;

    // Addressing modes
    uint8_t IMP(); uint8_t IMM(); uint8_t ZP0(); uint8_t ZPX();
    uint8_t ZPY(); uint8_t REL(); uint8_t ABS(); uint8_t ABX();
    uint8_t ABY(); uint8_t IND(); uint8_t IZX(); uint8_t IZY();

    // Opcodes
    uint8_t ADC(); uint8_t AND(); uint8_t ASL(); uint8_t BCC();
    uint8_t BCS(); uint8_t BEQ(); uint8_t BIT(); uint8_t BMI();
    uint8_t BNE(); uint8_t BPL(); uint8_t BRK(); uint8_t BVC();
    uint8_t BVS(); uint8_t CLC(); uint8_t CLD(); uint8_t CLI();
    uint8_t CLV(); uint8_t CMP(); uint8_t CPX(); uint8_t CPY();
    uint8_t DEC(); uint8_t DEX(); uint8_t DEY(); uint8_t EOR();
    uint8_t INC(); uint8_t INX(); uint8_t INY(); uint8_t JMP();
    uint8_t JSR(); uint8_t LDA(); uint8_t LDX(); uint8_t LDY();
    uint8_t LSR(); uint8_t NOP(); uint8_t ORA(); uint8_t PHA();
    uint8_t PHP(); uint8_t PLA(); uint8_t PLP(); uint8_t ROL();
    uint8_t ROR(); uint8_t RTI(); uint8_t RTS(); uint8_t SBC();
    uint8_t SEC(); uint8_t SED(); uint8_t SEI(); uint8_t STA();
    uint8_t STX(); uint8_t STY(); uint8_t TAX(); uint8_t TAY();
    uint8_t TSX(); uint8_t TXA(); uint8_t TXS(); uint8_t TYA();

    // Illegal / NOP fallback
    uint8_t XXX();

    uint8_t fetch();
};

#endif // CPU_HPP
