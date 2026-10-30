#ifndef BUS_HPP
#define BUS_HPP

#include <cstdint>
#include <memory>
#include <array>

class CPU;
class PPU;
class APU;
class Cartridge;
class Controller;

class Bus {
public:
    Bus();
    ~Bus();

    void connectCPU(CPU* cpu);
    void connectPPU(PPU* ppu);
    void connectAPU(APU* apu);
    void insertCartridge(std::shared_ptr<Cartridge> cartridge);
    void connectControllers(Controller* pad1, Controller* pad2);

    uint8_t cpuRead(uint16_t addr, bool readOnly = false);
    void cpuWrite(uint16_t addr, uint8_t data);

    uint8_t ppuRead(uint16_t addr);
    void ppuWrite(uint16_t addr, uint8_t data);

    void reset();
    void clock();

    std::array<uint8_t, 2048> ram{};

private:
    CPU* m_cpu = nullptr;
    PPU* m_ppu = nullptr;
    APU* m_apu = nullptr;
    std::shared_ptr<Cartridge> m_cartridge = nullptr;
    Controller* m_controller1 = nullptr;
    Controller* m_controller2 = nullptr;

    uint32_t m_systemClockCounter = 0;
    uint8_t m_dmaPage = 0x00;
    uint8_t m_dmaAddr = 0x00;
    uint8_t m_dmaData = 0x00;
    bool m_dmaTransfer = false;
    bool m_dmaDummy = true;
};

#endif // BUS_HPP
