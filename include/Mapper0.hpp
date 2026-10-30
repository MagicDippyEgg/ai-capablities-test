#ifndef MAPPER0_HPP
#define MAPPER0_HPP

#include "Mapper.hpp"

class Mapper0 : public Mapper {
public:
    Mapper0(uint8_t prgBanks, uint8_t chrBanks);
    ~Mapper0() override = default;

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;
};

#endif // MAPPER0_HPP
