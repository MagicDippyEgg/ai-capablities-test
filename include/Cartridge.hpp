#ifndef CARTRIDGE_HPP
#define CARTRIDGE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "Mapper.hpp"

class Cartridge {
public:
    Cartridge(const std::string& filename);
    ~Cartridge() = default;

    bool isLoaded() const { return m_loaded; }

    bool cpuRead(uint16_t addr, uint8_t &data);
    bool cpuWrite(uint16_t addr, uint8_t data);

    bool ppuRead(uint16_t addr, uint8_t &data);
    bool ppuWrite(uint16_t addr, uint8_t data);

    MirrorMode getMirrorMode() const;

private:
    bool m_loaded = false;
    uint8_t m_prgBanks = 0;
    uint8_t m_chrBanks = 0;

    std::vector<uint8_t> m_prgMemory;
    std::vector<uint8_t> m_chrMemory;

    std::shared_ptr<Mapper> m_mapper;
    MirrorMode m_mirrorMode = MirrorMode::HORIZONTAL;
};

#endif // CARTRIDGE_HPP
