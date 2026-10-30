#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <cstdint>

class Controller {
public:
    Controller() = default;
    ~Controller() = default;

    enum Button {
        RIGHT  = (1 << 0),
        LEFT   = (1 << 1),
        DOWN   = (1 << 2),
        UP     = (1 << 3),
        START  = (1 << 4),
        SELECT = (1 << 5),
        B      = (1 << 6),
        A      = (1 << 7)
    };

    void write(uint8_t data) {
        m_strobe = (data & 0x01);
        if (m_strobe) {
            m_shiftState = m_buttons;
        }
    }

    uint8_t read() {
        uint8_t result = 0;
        if (m_strobe) {
            result = (m_buttons & 0x80) ? 1 : 0;
        } else {
            result = (m_shiftState & 0x80) ? 1 : 0;
            m_shiftState <<= 1;
        }
        return result;
    }

    void setButton(Button btn, bool pressed) {
        if (pressed) {
            m_buttons |= btn;
        } else {
            m_buttons &= ~btn;
        }
    }

private:
    uint8_t m_buttons = 0x00;
    uint8_t m_shiftState = 0x00;
    bool m_strobe = false;
};

#endif // CONTROLLER_HPP
