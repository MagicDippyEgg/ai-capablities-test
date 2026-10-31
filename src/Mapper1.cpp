#include "Mapper1.hpp"

Mapper1::Mapper1(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

bool Mapper1::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        uint8_t prgMode = (m_controlRegister >> 2) & 0x03;

        if (prgMode == 0 || prgMode == 1) { // 32 KB mode
            uint8_t bank = (m_prgBank & 0x0E);
            mapped_addr = (bank * 0x4000) + (addr & 0x7FFF);
            return true;
        } else if (prgMode == 2) { // Fix first bank at $8000, switch $C000
            if (addr < 0xC000) {
                mapped_addr = addr & 0x3FFF;
            } else {
                mapped_addr = ((m_prgBank & 0x0F) * 0x4000) + (addr & 0x3FFF);
            }
            return true;
        } else if (prgMode == 3) { // Fix last bank at $C000, switch $8000
            if (addr < 0xC000) {
                mapped_addr = ((m_prgBank & 0x0F) * 0x4000) + (addr & 0x3FFF);
            } else {
                mapped_addr = ((m_prgBanks - 1) * 0x4000) + (addr & 0x3FFF);
            }
            return true;
        }
    }
    return false;
}

bool Mapper1::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        if (data & 0x80) { // Reset shift register
            m_shiftRegister = 0x00;
            m_writeCount = 0;
            m_controlRegister |= 0x0C;
        } else {
            m_shiftRegister = ((data & 0x01) << 4) | (m_shiftRegister >> 1);
            m_writeCount++;

            if (m_writeCount == 5) {
                uint8_t targetReg = (addr >> 13) & 0x03;

                switch (targetReg) {
                case 0: // Control
                    m_controlRegister = m_shiftRegister & 0x1F;
                    break;
                case 1: // CHR Bank 0
                    m_chrBank0 = m_shiftRegister & 0x1F;
                    break;
                case 2: // CHR Bank 1
                    m_chrBank1 = m_shiftRegister & 0x1F;
                    break;
                case 3: // PRG Bank
                    m_prgBank = m_shiftRegister & 0x0F;
                    break;
                }

                m_shiftRegister = 0x00;
                m_writeCount = 0;
            }
        }
    }
    return false;
}

bool Mapper1::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) { // CHR RAM
            mapped_addr = addr;
            return true;
        }

        if (m_controlRegister & 0x10) { // 4 KB mode
            if (addr < 0x1000) {
                mapped_addr = (m_chrBank0 * 0x1000) + (addr & 0x0FFF);
            } else {
                mapped_addr = (m_chrBank1 * 0x1000) + (addr & 0x0FFF);
            }
        } else { // 8 KB mode
            uint8_t bank = m_chrBank0 & 0x1E;
            mapped_addr = (bank * 0x1000) + (addr & 0x1FFF);
        }
        return true;
    }
    return false;
}

bool Mapper1::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) {
            mapped_addr = addr;
            return true;
        }
    }
    return false;
}

MirrorMode Mapper1::mirrorMode() const {
    switch (m_controlRegister & 0x03) {
    case 0: return MirrorMode::SINGLE_SCREEN_LOWER;
    case 1: return MirrorMode::SINGLE_SCREEN_UPPER;
    case 2: return MirrorMode::VERTICAL;
    case 3: return MirrorMode::HORIZONTAL;
    }
    return MirrorMode::HORIZONTAL;
}
