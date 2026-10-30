#ifndef APU_HPP
#define APU_HPP

#include <cstdint>
#include <vector>

class APU {
public:
    APU();
    ~APU();

    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr);

    void clock();
    void reset();

    float getOutputSample() const;

private:
    uint32_t m_clockCounter = 0;
    uint8_t m_frameSequencerStep = 0;

    // Pulse 1 & 2
    struct PulseChannel {
        bool enabled = false;
        uint8_t duty = 0;
        uint8_t lengthCounter = 0;
        uint16_t timer = 0;
        uint16_t timerValue = 0;
        uint8_t volume = 0;
        bool constantVolume = false;
        uint8_t envelope = 0;
        uint8_t sequencePos = 0;
    } m_pulse1, m_pulse2;

    // Triangle
    struct TriangleChannel {
        bool enabled = false;
        uint8_t lengthCounter = 0;
        uint16_t timer = 0;
        uint16_t timerValue = 0;
        uint8_t sequencePos = 0;
    } m_triangle;

    // Noise
    struct NoiseChannel {
        bool enabled = false;
        uint8_t lengthCounter = 0;
        uint16_t timer = 0;
        uint16_t timerValue = 0;
        uint16_t shiftRegister = 1;
        uint8_t volume = 0;
        bool mode = false;
    } m_noise;
};

#endif // APU_HPP
