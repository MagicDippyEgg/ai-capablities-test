#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <cstdint>
#include <vector>

enum class MirrorMode {
    HORIZONTAL,
    VERTICAL,
    SINGLE_SCREEN_LOWER,
    SINGLE_SCREEN_UPPER,
    FOUR_SCREEN
};

class Mapper {
public:
    Mapper(uint8_t prgBanks, uint8_t chrBanks)
        : m_prgBanks(prgBanks), m_chrBanks(chrBanks) {}
    virtual ~Mapper() = default;

    virtual bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) = 0;
    virtual bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) = 0;
    virtual bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) = 0;
    virtual bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) = 0;

    virtual MirrorMode mirrorMode() const { return MirrorMode::HORIZONTAL; }
    virtual bool irqActive() const { return false; }
    virtual void irqClear() {}
    virtual void scanline() {}

protected:
    uint8_t m_prgBanks = 0;
    uint8_t m_chrBanks = 0;
};

#endif // MAPPER_HPP
