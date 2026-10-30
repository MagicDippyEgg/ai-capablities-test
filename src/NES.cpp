#include "NES.hpp"

NES::NES() {
    m_bus.connectCPU(&m_cpu);
    m_bus.connectPPU(&m_ppu);
    m_bus.connectAPU(&m_apu);
    m_bus.connectControllers(&m_controller1, &m_controller2);
    m_cpu.connectBus(&m_bus);
}

NES::~NES() = default;

bool NES::loadCartridge(const std::string& path) {
    m_cartridge = std::make_shared<Cartridge>(path);
    if (m_cartridge->isLoaded()) {
        m_bus.insertCartridge(m_cartridge);
        reset();
        return true;
    }
    return false;
}

void NES::reset() {
    m_bus.reset();
}

void NES::clock() {
    m_bus.clock();
}
