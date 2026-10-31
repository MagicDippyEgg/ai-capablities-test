#ifndef MAPPER4_HPP
#define MAPPER4_HPP

#include "Mapper.hpp"
#include <array>

class Mapper4 : public Mapper {
public:
    Mapper4(uint8_t prgBanks, uint8_t chrBanks);
    ~Mapper4() override = default;

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;

    MirrorMode mirrorMode() const override;
    bool irqActive() const override;
    void irqClear() override;
    void scanline() override;

private:
    uint8_t m_targetRegister = 0;
    bool m_prgBankMode = false;
    bool m_chrInversion = false;

    std::array<uint8_t, 8> m_registers{};
    std::array<uint32_t, 8> m_chrBankOffsets{};
    std::array<uint32_t, 4> m_prgBankOffsets{};

    MirrorMode m_mirrorMode = MirrorMode::HORIZONTAL;

    bool m_irqActive = false;
    bool m_irqEnable = false;
    bool m_irqReload = false;
    uint8_t m_irqCounter = 0;
    uint8_t m_irqLatch = 0;

    void updateBankOffsets();
};

#endif // MAPPER4_HPP
