#include "CPU.hpp"
#include "Bus.hpp"
#include <iostream>

CPU::CPU() {
    // 256 Opcode matrix
    m_lookup = {
        { "BRK", &CPU::BRK, &CPU::IMM, 7 }, { "ORA", &CPU::ORA, &CPU::IZX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 3 }, { "ORA", &CPU::ORA, &CPU::ZP0, 3 }, { "ASL", &CPU::ASL, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "PHP", &CPU::PHP, &CPU::IMP, 3 }, { "ORA", &CPU::ORA, &CPU::IMM, 2 }, { "ASL", &CPU::ASL, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "ORA", &CPU::ORA, &CPU::ABS, 4 }, { "ASL", &CPU::ASL, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BPL", &CPU::BPL, &CPU::REL, 2 }, { "ORA", &CPU::ORA, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "ORA", &CPU::ORA, &CPU::ZPX, 4 }, { "ASL", &CPU::ASL, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "CLC", &CPU::CLC, &CPU::IMP, 2 }, { "ORA", &CPU::ORA, &CPU::ABY, 4 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "ORA", &CPU::ORA, &CPU::ABX, 4 }, { "ASL", &CPU::ASL, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 },
        { "JSR", &CPU::JSR, &CPU::ABS, 6 }, { "AND", &CPU::AND, &CPU::IZX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "BIT", &CPU::BIT, &CPU::ZP0, 3 }, { "AND", &CPU::AND, &CPU::ZP0, 3 }, { "ROL", &CPU::ROL, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "PLP", &CPU::PLP, &CPU::IMP, 4 }, { "AND", &CPU::AND, &CPU::IMM, 2 }, { "ROL", &CPU::ROL, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "BIT", &CPU::BIT, &CPU::ABS, 4 }, { "AND", &CPU::AND, &CPU::ABS, 4 }, { "ROL", &CPU::ROL, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BMI", &CPU::BMI, &CPU::REL, 2 }, { "AND", &CPU::AND, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "AND", &CPU::AND, &CPU::ZPX, 4 }, { "ROL", &CPU::ROL, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "SEC", &CPU::SEC, &CPU::IMP, 2 }, { "AND", &CPU::AND, &CPU::ABY, 4 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "AND", &CPU::AND, &CPU::ABX, 4 }, { "ROL", &CPU::ROL, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 },
        { "RTI", &CPU::RTI, &CPU::IMP, 6 }, { "EOR", &CPU::EOR, &CPU::IZX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 3 }, { "EOR", &CPU::EOR, &CPU::ZP0, 3 }, { "LSR", &CPU::LSR, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "PHA", &CPU::PHA, &CPU::IMP, 3 }, { "EOR", &CPU::EOR, &CPU::IMM, 2 }, { "LSR", &CPU::LSR, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "JMP", &CPU::JMP, &CPU::ABS, 3 }, { "EOR", &CPU::EOR, &CPU::ABS, 4 }, { "LSR", &CPU::LSR, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BVC", &CPU::BVC, &CPU::REL, 2 }, { "EOR", &CPU::EOR, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "EOR", &CPU::EOR, &CPU::ZPX, 4 }, { "LSR", &CPU::LSR, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "CLI", &CPU::CLI, &CPU::IMP, 2 }, { "EOR", &CPU::EOR, &CPU::ABY, 4 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "EOR", &CPU::EOR, &CPU::ABX, 4 }, { "LSR", &CPU::LSR, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 },
        { "RTS", &CPU::RTS, &CPU::IMP, 6 }, { "ADC", &CPU::ADC, &CPU::IZX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 3 }, { "ADC", &CPU::ADC, &CPU::ZP0, 3 }, { "ROR", &CPU::ROR, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "PLA", &CPU::PLA, &CPU::IMP, 4 }, { "ADC", &CPU::ADC, &CPU::IMM, 2 }, { "ROR", &CPU::ROR, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "JMP", &CPU::JMP, &CPU::IND, 5 }, { "ADC", &CPU::ADC, &CPU::ABS, 4 }, { "ROR", &CPU::ROR, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BVS", &CPU::BVS, &CPU::REL, 2 }, { "ADC", &CPU::ADC, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "ADC", &CPU::ADC, &CPU::ZPX, 4 }, { "ROR", &CPU::ROR, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "SEI", &CPU::SEI, &CPU::IMP, 2 }, { "ADC", &CPU::ADC, &CPU::ABY, 4 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "ADC", &CPU::ADC, &CPU::ABX, 4 }, { "ROR", &CPU::ROR, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 },
        { "???", &CPU::NOP, &CPU::IMP, 2 }, { "STA", &CPU::STA, &CPU::IZX, 6 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "STY", &CPU::STY, &CPU::ZP0, 3 }, { "STA", &CPU::STA, &CPU::ZP0, 3 }, { "STX", &CPU::STX, &CPU::ZP0, 3 }, { "???", &CPU::XXX, &CPU::IMP, 3 }, { "DEY", &CPU::DEY, &CPU::IMP, 2 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "TXA", &CPU::TXA, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "STY", &CPU::STY, &CPU::ABS, 4 }, { "STA", &CPU::STA, &CPU::ABS, 4 }, { "STX", &CPU::STX, &CPU::ABS, 4 }, { "???", &CPU::XXX, &CPU::IMP, 4 },
        { "BCC", &CPU::BCC, &CPU::REL, 2 }, { "STA", &CPU::STA, &CPU::IZY, 6 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "STY", &CPU::STY, &CPU::ZPX, 4 }, { "STA", &CPU::STA, &CPU::ZPX, 4 }, { "STX", &CPU::STX, &CPU::ZPY, 4 }, { "???", &CPU::XXX, &CPU::IMP, 4 }, { "TYA", &CPU::TYA, &CPU::IMP, 2 }, { "STA", &CPU::STA, &CPU::ABY, 5 }, { "TXS", &CPU::TXS, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "???", &CPU::NOP, &CPU::IMP, 5 }, { "STA", &CPU::STA, &CPU::ABX, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 },
        { "LDY", &CPU::LDY, &CPU::IMM, 2 }, { "LDA", &CPU::LDA, &CPU::IZX, 6 }, { "LDX", &CPU::LDX, &CPU::IMM, 2 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "LDY", &CPU::LDY, &CPU::ZP0, 3 }, { "LDA", &CPU::LDA, &CPU::ZP0, 3 }, { "LDX", &CPU::LDX, &CPU::ZP0, 3 }, { "???", &CPU::XXX, &CPU::IMP, 3 }, { "TAY", &CPU::TAY, &CPU::IMP, 2 }, { "LDA", &CPU::LDA, &CPU::IMM, 2 }, { "TAX", &CPU::TAX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "LDY", &CPU::LDY, &CPU::ABS, 4 }, { "LDA", &CPU::LDA, &CPU::ABS, 4 }, { "LDX", &CPU::LDX, &CPU::ABS, 4 }, { "???", &CPU::XXX, &CPU::IMP, 4 },
        { "BCS", &CPU::BCS, &CPU::REL, 2 }, { "LDA", &CPU::LDA, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "LDY", &CPU::LDY, &CPU::ZPX, 4 }, { "LDA", &CPU::LDA, &CPU::ZPX, 4 }, { "LDX", &CPU::LDX, &CPU::ZPY, 4 }, { "???", &CPU::XXX, &CPU::IMP, 4 }, { "CLV", &CPU::CLV, &CPU::IMP, 2 }, { "LDA", &CPU::LDA, &CPU::ABY, 4 }, { "TSX", &CPU::TSX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 4 }, { "LDY", &CPU::LDY, &CPU::ABX, 4 }, { "LDA", &CPU::LDA, &CPU::ABX, 4 }, { "LDX", &CPU::LDX, &CPU::ABY, 4 }, { "???", &CPU::XXX, &CPU::IMP, 4 },
        { "CPY", &CPU::CPY, &CPU::IMM, 2 }, { "CMP", &CPU::CMP, &CPU::IZX, 6 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "CPY", &CPU::CPY, &CPU::ZP0, 3 }, { "CMP", &CPU::CMP, &CPU::ZP0, 3 }, { "DEC", &CPU::DEC, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "INY", &CPU::INY, &CPU::IMP, 2 }, { "CMP", &CPU::CMP, &CPU::IMM, 2 }, { "DEX", &CPU::DEX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "CPY", &CPU::CPY, &CPU::ABS, 4 }, { "CMP", &CPU::CMP, &CPU::ABS, 4 }, { "DEC", &CPU::DEC, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BNE", &CPU::BNE, &CPU::REL, 2 }, { "CMP", &CPU::CMP, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "CMP", &CPU::CMP, &CPU::ZPX, 4 }, { "DEC", &CPU::DEC, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "CLD", &CPU::CLD, &CPU::IMP, 2 }, { "CMP", &CPU::CMP, &CPU::ABY, 4 }, { "NOP", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "CMP", &CPU::CMP, &CPU::ABX, 4 }, { "DEC", &CPU::DEC, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 },
        { "CPX", &CPU::CPX, &CPU::IMM, 2 }, { "SBC", &CPU::SBC, &CPU::IZX, 6 }, { "???", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "CPX", &CPU::CPX, &CPU::ZP0, 3 }, { "SBC", &CPU::SBC, &CPU::ZP0, 3 }, { "INC", &CPU::INC, &CPU::ZP0, 5 }, { "???", &CPU::XXX, &CPU::IMP, 5 }, { "INX", &CPU::INX, &CPU::IMP, 2 }, { "SBC", &CPU::SBC, &CPU::IMM, 2 }, { "NOP", &CPU::NOP, &CPU::IMP, 2 }, { "SBC", &CPU::SBC, &CPU::IMP, 2 }, { "CPX", &CPU::CPX, &CPU::ABS, 4 }, { "SBC", &CPU::SBC, &CPU::ABS, 4 }, { "INC", &CPU::INC, &CPU::ABS, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 },
        { "BEQ", &CPU::BEQ, &CPU::REL, 2 }, { "SBC", &CPU::SBC, &CPU::IZY, 5 }, { "???", &CPU::XXX, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 8 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "SBC", &CPU::SBC, &CPU::ZPX, 4 }, { "INC", &CPU::INC, &CPU::ZPX, 6 }, { "???", &CPU::XXX, &CPU::IMP, 6 }, { "SED", &CPU::SED, &CPU::IMP, 2 }, { "SBC", &CPU::SBC, &CPU::ABY, 4 }, { "NOP", &CPU::NOP, &CPU::IMP, 2 }, { "???", &CPU::XXX, &CPU::IMP, 7 }, { "???", &CPU::NOP, &CPU::IMP, 4 }, { "SBC", &CPU::SBC, &CPU::ABX, 4 }, { "INC", &CPU::INC, &CPU::ABX, 7 }, { "???", &CPU::XXX, &CPU::IMP, 7 }
    };
}

CPU::~CPU() = default;

uint8_t CPU::read(uint16_t addr) {
    return m_bus ? m_bus->cpuRead(addr, false) : 0x00;
}

void CPU::write(uint16_t addr, uint8_t data) {
    if (m_bus) m_bus->cpuWrite(addr, data);
}

void CPU::clock() {
    if (m_cycles == 0) {
        if (m_nmiPending) {
            m_nmiPending = false;

            write(0x0100 + stp, (pc >> 8) & 0x00FF);
            stp--;
            write(0x0100 + stp, pc & 0x00FF);
            stp--;

            setFlag(B, false);
            setFlag(U, true);
            setFlag(I, true);
            write(0x0100 + stp, status);
            stp--;

            m_addrAbs = 0xFFFA;
            uint16_t lo = read(m_addrAbs + 0);
            uint16_t hi = read(m_addrAbs + 1);
            pc = (hi << 8) | lo;

            m_cycles = 7;
        } else {
            m_opcode = read(pc);
            setFlag(U, true);
            pc++;

            Instruction inst = m_lookup[m_opcode];
            m_cycles = inst.cycles;

            uint8_t additionalCycle1 = (this->*inst.addrmode)();
            uint8_t additionalCycle2 = (this->*inst.operate)();

            m_cycles += (additionalCycle1 & additionalCycle2);
            setFlag(U, true);
        }
    }

    m_clockCount++;
    m_cycles--;
}

void CPU::reset() {
    m_addrAbs = 0xFFFC;
    uint16_t lo = read(m_addrAbs + 0);
    uint16_t hi = read(m_addrAbs + 1);
    pc = (hi << 8) | lo;

    a = 0;
    x = 0;
    y = 0;
    stp = 0xFD;
    status = 0x00 | U | I;

    m_addrRel = 0x0000;
    m_addrAbs = 0x0000;
    m_fetched = 0x00;
    m_nmiPending = false;

    m_cycles = 8;
}

void CPU::irq() {
    if (getFlag(I) == 0) {
        write(0x0100 + stp, (pc >> 8) & 0x00FF);
        stp--;
        write(0x0100 + stp, pc & 0x00FF);
        stp--;

        setFlag(B, false);
        setFlag(U, true);
        setFlag(I, true);
        write(0x0100 + stp, status);
        stp--;

        m_addrAbs = 0xFFFE;
        uint16_t lo = read(m_addrAbs + 0);
        uint16_t hi = read(m_addrAbs + 1);
        pc = (hi << 8) | lo;

        m_cycles = 7;
    }
}

uint8_t CPU::fetch() {
    if (m_lookup[m_opcode].addrmode != &CPU::IMP) {
        m_fetched = read(m_addrAbs);
    }
    return m_fetched;
}

// Addressing Modes
uint8_t CPU::IMP() {
    m_fetched = a;
    return 0;
}

uint8_t CPU::IMM() {
    m_addrAbs = pc++;
    return 0;
}

uint8_t CPU::ZP0() {
    m_addrAbs = read(pc);
    pc++;
    m_addrAbs &= 0x00FF;
    return 0;
}

uint8_t CPU::ZPX() {
    m_addrAbs = (read(pc) + x);
    pc++;
    m_addrAbs &= 0x00FF;
    return 0;
}

uint8_t CPU::ZPY() {
    m_addrAbs = (read(pc) + y);
    pc++;
    m_addrAbs &= 0x00FF;
    return 0;
}

uint8_t CPU::REL() {
    m_addrRel = read(pc);
    pc++;
    if (m_addrRel & 0x80) {
        m_addrRel |= 0xFF00;
    }
    return 0;
}

uint8_t CPU::ABS() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;
    m_addrAbs = (hi << 8) | lo;
    return 0;
}

uint8_t CPU::ABX() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;
    m_addrAbs = (hi << 8) | lo;
    m_addrAbs += x;

    if ((m_addrAbs & 0xFF00) != (hi << 8)) {
        return 1; // Page boundary crossed
    }
    return 0;
}

uint8_t CPU::ABY() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;
    m_addrAbs = (hi << 8) | lo;
    m_addrAbs += y;

    if ((m_addrAbs & 0xFF00) != (hi << 8)) {
        return 1; // Page boundary crossed
    }
    return 0;
}

uint8_t CPU::IND() {
    uint16_t ptr_lo = read(pc);
    pc++;
    uint16_t ptr_hi = read(pc);
    pc++;
    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF) { // Hardware page boundary bug simulation
        m_addrAbs = (read(ptr & 0xFF00) << 8) | read(ptr + 0);
    } else {
        m_addrAbs = (read(ptr + 1) << 8) | read(ptr + 0);
    }
    return 0;
}

uint8_t CPU::IZX() {
    uint16_t t = read(pc);
    pc++;
    uint16_t lo = read((uint16_t)(t + (uint16_t)x) & 0x00FF);
    uint16_t hi = read((uint16_t)(t + (uint16_t)x + 1) & 0x00FF);
    m_addrAbs = (hi << 8) | lo;
    return 0;
}

uint8_t CPU::IZY() {
    uint16_t t = read(pc);
    pc++;
    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);
    m_addrAbs = (hi << 8) | lo;
    m_addrAbs += y;

    if ((m_addrAbs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// Opcodes
uint8_t CPU::ADC() {
    fetch();
    uint16_t temp = (uint16_t)a + (uint16_t)m_fetched + (uint16_t)getFlag(C);
    setFlag(C, temp > 255);
    setFlag(Z, (temp & 0x00FF) == 0);
    setFlag(V, (~((uint16_t)a ^ (uint16_t)m_fetched) & ((uint16_t)a ^ (uint16_t)temp)) & 0x0080);
    setFlag(N, temp & 0x80);
    a = temp & 0x00FF;
    return 1;
}

uint8_t CPU::SBC() {
    fetch();
    uint16_t value = ((uint16_t)m_fetched) ^ 0x00FF;
    uint16_t temp = (uint16_t)a + value + (uint16_t)getFlag(C);
    setFlag(C, temp & 0xFF00);
    setFlag(Z, (temp & 0x00FF) == 0);
    setFlag(V, (temp ^ (uint16_t)a) & (temp ^ value) & 0x0080);
    setFlag(N, temp & 0x0080);
    a = temp & 0x00FF;
    return 1;
}

uint8_t CPU::AND() {
    fetch();
    a = a & m_fetched;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 1;
}

uint8_t CPU::ASL() {
    fetch();
    uint16_t temp = (uint16_t)m_fetched << 1;
    setFlag(C, (temp & 0xFF00) > 0);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x80);
    if (m_lookup[m_opcode].addrmode == &CPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(m_addrAbs, temp & 0x00FF);
    }
    return 0;
}

uint8_t CPU::BCC() {
    if (getFlag(C) == 0) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BCS() {
    if (getFlag(C) == 1) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BEQ() {
    if (getFlag(Z) == 1) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BIT() {
    fetch();
    uint8_t temp = a & m_fetched;
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, m_fetched & (1 << 7));
    setFlag(V, m_fetched & (1 << 6));
    return 0;
}

uint8_t CPU::BMI() {
    if (getFlag(N) == 1) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BNE() {
    if (getFlag(Z) == 0) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BPL() {
    if (getFlag(N) == 0) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BRK() {
    pc++;
    setFlag(I, true);
    write(0x0100 + stp, (pc >> 8) & 0x00FF);
    stp--;
    write(0x0100 + stp, pc & 0x00FF);
    stp--;

    setFlag(B, true);
    write(0x0100 + stp, status);
    stp--;
    setFlag(B, false);

    pc = (uint16_t)read(0xFFFE) | ((uint16_t)read(0xFFFF) << 8);
    return 0;
}

uint8_t CPU::BVC() {
    if (getFlag(V) == 0) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::BVS() {
    if (getFlag(V) == 1) {
        m_cycles++;
        m_addrAbs = pc + m_addrRel;
        if ((m_addrAbs & 0xFF00) != (pc & 0xFF00)) {
            m_cycles++;
        }
        pc = m_addrAbs;
    }
    return 0;
}

uint8_t CPU::CLC() { setFlag(C, false); return 0; }
uint8_t CPU::CLD() { setFlag(D, false); return 0; }
uint8_t CPU::CLI() { setFlag(I, false); return 0; }
uint8_t CPU::CLV() { setFlag(V, false); return 0; }

uint8_t CPU::CMP() {
    fetch();
    uint16_t temp = (uint16_t)a - (uint16_t)m_fetched;
    setFlag(C, a >= m_fetched);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    return 1;
}

uint8_t CPU::CPX() {
    fetch();
    uint16_t temp = (uint16_t)x - (uint16_t)m_fetched;
    setFlag(C, x >= m_fetched);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    return 0;
}

uint8_t CPU::CPY() {
    fetch();
    uint16_t temp = (uint16_t)y - (uint16_t)m_fetched;
    setFlag(C, y >= m_fetched);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    return 0;
}

uint8_t CPU::DEC() {
    fetch();
    uint16_t temp = m_fetched - 1;
    write(m_addrAbs, temp & 0x00FF);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    return 0;
}

uint8_t CPU::DEX() {
    x--;
    setFlag(Z, x == 0x00);
    setFlag(N, x & 0x80);
    return 0;
}

uint8_t CPU::DEY() {
    y--;
    setFlag(Z, y == 0x00);
    setFlag(N, y & 0x80);
    return 0;
}

uint8_t CPU::EOR() {
    fetch();
    a = a ^ m_fetched;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 1;
}

uint8_t CPU::INC() {
    fetch();
    uint16_t temp = m_fetched + 1;
    write(m_addrAbs, temp & 0x00FF);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    return 0;
}

uint8_t CPU::INX() {
    x++;
    setFlag(Z, x == 0x00);
    setFlag(N, x & 0x80);
    return 0;
}

uint8_t CPU::INY() {
    y++;
    setFlag(Z, y == 0x00);
    setFlag(N, y & 0x80);
    return 0;
}

uint8_t CPU::JMP() {
    pc = m_addrAbs;
    return 0;
}

uint8_t CPU::JSR() {
    pc--;
    write(0x0100 + stp, (pc >> 8) & 0x00FF);
    stp--;
    write(0x0100 + stp, pc & 0x00FF);
    stp--;
    pc = m_addrAbs;
    return 0;
}

uint8_t CPU::LDA() {
    fetch();
    a = m_fetched;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 1;
}

uint8_t CPU::LDX() {
    fetch();
    x = m_fetched;
    setFlag(Z, x == 0x00);
    setFlag(N, x & 0x80);
    return 1;
}

uint8_t CPU::LDY() {
    fetch();
    y = m_fetched;
    setFlag(Z, y == 0x00);
    setFlag(N, y & 0x80);
    return 1;
}

uint8_t CPU::LSR() {
    fetch();
    setFlag(C, m_fetched & 0x0001);
    uint8_t temp = m_fetched >> 1;
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    if (m_lookup[m_opcode].addrmode == &CPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(m_addrAbs, temp & 0x00FF);
    }
    return 0;
}

uint8_t CPU::NOP() {
    switch (m_opcode) {
    case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC:
        return 1;
    }
    return 0;
}

uint8_t CPU::ORA() {
    fetch();
    a = a | m_fetched;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 1;
}

uint8_t CPU::PHA() {
    write(0x0100 + stp, a);
    stp--;
    return 0;
}

uint8_t CPU::PHP() {
    write(0x0100 + stp, status | B | U);
    setFlag(B, false);
    setFlag(U, false);
    stp--;
    return 0;
}

uint8_t CPU::PLA() {
    stp++;
    a = read(0x0100 + stp);
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 0;
}

uint8_t CPU::PLP() {
    stp++;
    status = read(0x0100 + stp);
    setFlag(U, true);
    return 0;
}

uint8_t CPU::ROL() {
    fetch();
    uint16_t temp = (uint16_t)(m_fetched << 1) | getFlag(C);
    setFlag(C, temp & 0xFF00);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    if (m_lookup[m_opcode].addrmode == &CPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(m_addrAbs, temp & 0x00FF);
    }
    return 0;
}

uint8_t CPU::ROR() {
    fetch();
    uint16_t temp = (uint16_t)(getFlag(C) << 7) | (m_fetched >> 1);
    setFlag(C, m_fetched & 0x01);
    setFlag(Z, (temp & 0x00FF) == 0x00);
    setFlag(N, temp & 0x0080);
    if (m_lookup[m_opcode].addrmode == &CPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(m_addrAbs, temp & 0x00FF);
    }
    return 0;
}

uint8_t CPU::RTI() {
    stp++;
    status = read(0x0100 + stp);
    status &= ~B;
    status &= ~U;

    stp++;
    pc = (uint16_t)read(0x0100 + stp);
    stp++;
    pc |= (uint16_t)read(0x0100 + stp) << 8;
    return 0;
}

uint8_t CPU::RTS() {
    stp++;
    pc = (uint16_t)read(0x0100 + stp);
    stp++;
    pc |= (uint16_t)read(0x0100 + stp) << 8;
    pc++;
    return 0;
}

uint8_t CPU::SEC() { setFlag(C, true); return 0; }
uint8_t CPU::SED() { setFlag(D, true); return 0; }
uint8_t CPU::SEI() { setFlag(I, true); return 0; }

uint8_t CPU::STA() {
    write(m_addrAbs, a);
    return 0;
}

uint8_t CPU::STX() {
    write(m_addrAbs, x);
    return 0;
}

uint8_t CPU::STY() {
    write(m_addrAbs, y);
    return 0;
}

uint8_t CPU::TAX() {
    x = a;
    setFlag(Z, x == 0x00);
    setFlag(N, x & 0x80);
    return 0;
}

uint8_t CPU::TAY() {
    y = a;
    setFlag(Z, y == 0x00);
    setFlag(N, y & 0x80);
    return 0;
}

uint8_t CPU::TSX() {
    x = stp;
    setFlag(Z, x == 0x00);
    setFlag(N, x & 0x80);
    return 0;
}

uint8_t CPU::TXA() {
    a = x;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 0;
}

uint8_t CPU::TXS() {
    stp = x;
    return 0;
}

uint8_t CPU::TYA() {
    a = y;
    setFlag(Z, a == 0x00);
    setFlag(N, a & 0x80);
    return 0;
}

uint8_t CPU::XXX() {
    return 0;
}
