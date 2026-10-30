#ifndef NES_HPP
#define NES_HPP

#include <memory>
#include <string>
#include "Bus.hpp"
#include "CPU.hpp"
#include "PPU.hpp"
#include "APU.hpp"
#include "Cartridge.hpp"
#include "Controller.hpp"

class NES {
public:
    NES();
    ~NES();

    bool loadCartridge(const std::string& path);
    void reset();
    void clock();

    Bus& getBus() { return m_bus; }
    CPU& getCPU() { return m_cpu; }
    PPU& getPPU() { return m_ppu; }
    APU& getAPU() { return m_apu; }
    Controller& getController(int index) { return index == 0 ? m_controller1 : m_controller2; }

private:
    Bus m_bus;
    CPU m_cpu;
    PPU m_ppu;
    APU m_apu;
    std::shared_ptr<Cartridge> m_cartridge;
    Controller m_controller1;
    Controller m_controller2;
};

#endif // NES_HPP
