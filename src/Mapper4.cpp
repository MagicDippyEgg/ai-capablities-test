#include "Mapper4.hpp"

Mapper4::Mapper4(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
    m_registers.fill(0);
    updateBankOffsets();
}

void Mapper4::updateBankOffsets() {
    // CHR Banking (8 x 1KB)
    if (!m_chrInversion) {
        m_chrBankOffsets[0] = (m_registers[0] & 0xFE) * 0x0400;
        m_chrBankOffsets[1] = ((m_registers[0] & 0xFE) + 1) * 0x0400;
        m_chrBankOffsets[2] = (m_registers[1] & 0xFE) * 0x0400;
        m_chrBankOffsets[3] = ((m_registers[1] & 0xFE) + 1) * 0x0400;
        m_chrBankOffsets[4] = m_registers[2] * 0x0400;
        m_chrBankOffsets[5] = m_registers[3] * 0x0400;
        m_chrBankOffsets[6] = m_registers[4] * 0x0400;
        m_chrBankOffsets[7] = m_registers[5] * 0x0400;
    } else {
        m_chrBankOffsets[0] = m_registers[2] * 0x0400;
        m_chrBankOffsets[1] = m_registers[3] * 0x0400;
        m_chrBankOffsets[2] = m_registers[4] * 0x0400;
        m_chrBankOffsets[3] = m_registers[5] * 0x0400;
        m_chrBankOffsets[4] = (m_registers[0] & 0xFE) * 0x0400;
        m_chrBankOffsets[5] = ((m_registers[0] & 0xFE) + 1) * 0x0400;
        m_chrBankOffsets[6] = (m_registers[1] & 0xFE) * 0x0400;
        m_chrBankOffsets[7] = ((m_registers[1] & 0xFE) + 1) * 0x0400;
    }

    // PRG Banking (4 x 8KB)
    uint32_t total8kBanks = m_prgBanks * 2;
    uint32_t lastBank = (total8kBanks > 0) ? (total8kBanks - 1) : 0;
    uint32_t secondLastBank = (total8kBanks > 1) ? (total8kBanks - 2) : 0;

    if (!m_prgBankMode) {
        m_prgBankOffsets[0] = (m_registers[6] % total8kBanks) * 0x2000;
        m_prgBankOffsets[1] = (m_registers[7] % total8kBanks) * 0x2000;
        m_prgBankOffsets[2] = secondLastBank * 0x2000;
        m_prgBankOffsets[3] = lastBank * 0x2000;
    } else {
        m_prgBankOffsets[0] = secondLastBank * 0x2000;
        m_prgBankOffsets[1] = (m_registers[7] % total8kBanks) * 0x2000;
        m_prgBankOffsets[2] = (m_registers[6] % total8kBanks) * 0x2000;
        m_prgBankOffsets[3] = lastBank * 0x2000;
    }
}

bool Mapper4::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        mapped_addr = m_prgBankOffsets[0] + (addr & 0x1FFF);
        return true;
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
        mapped_addr = m_prgBankOffsets[1] + (addr & 0x1FFF);
        return true;
    } else if (addr >= 0xC000 && addr <= 0xDFFF) {
        mapped_addr = m_prgBankOffsets[2] + (addr & 0x1FFF);
        return true;
    } else if (addr >= 0xE000 && addr <= 0xFFFF) {
        mapped_addr = m_prgBankOffsets[3] + (addr & 0x1FFF);
        return true;
    }
    return false;
}

bool Mapper4::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) {
    (void)mapped_addr;
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        if (!(addr & 0x0001)) { // Even: Bank Select
            m_targetRegister = data & 0x07;
            m_prgBankMode = (data & 0x40) != 0;
            m_chrInversion = (data & 0x80) != 0;
        } else { // Odd: Bank Data
            m_registers[m_targetRegister] = data;
        }
        updateBankOffsets();
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (!(addr & 0x0001)) { // Even: Mirroring
            m_mirrorMode = (data & 0x01) ? MirrorMode::HORIZONTAL : MirrorMode::VERTICAL;
        } else { // Odd: PRG RAM Protect
            // Handled as enabled
        }
    } else if (addr >= 0xC000 && addr <= 0xDFFF) {
        if (!(addr & 0x0001)) { // Even: IRQ Latch
            m_irqLatch = data;
        } else { // Odd: IRQ Reload
            m_irqReload = true;
        }
    } else if (addr >= 0xE000 && addr <= 0xFFFF) {
        if (!(addr & 0x0001)) { // Even: IRQ Disable
            m_irqEnable = false;
            m_irqActive = false;
        } else { // Odd: IRQ Enable
            m_irqEnable = true;
        }
    }
    return false;
}

bool Mapper4::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        uint8_t slot = addr / 0x0400;
        mapped_addr = m_chrBankOffsets[slot] + (addr & 0x03FF);
        return true;
    }
    return false;
}

bool Mapper4::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) {
            mapped_addr = addr;
            return true;
        }
    }
    return false;
}

MirrorMode Mapper4::mirrorMode() const {
    return m_mirrorMode;
}

bool Mapper4::irqActive() const {
    return m_irqActive;
}

void Mapper4::irqClear() {
    m_irqActive = false;
}

void Mapper4::scanline() {
    if (m_irqCounter == 0 || m_irqReload) {
        m_irqCounter = m_irqLatch;
        m_irqReload = false;
    } else {
        m_irqCounter--;
    }

    if (m_irqCounter == 0 && m_irqEnable) {
        m_irqActive = true;
    }
}
