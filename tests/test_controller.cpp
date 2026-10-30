#include <iostream>
#include <cassert>
#include "Controller.hpp"

void testController_StrobeAndShift() {
    Controller pad;
    pad.setButton(Controller::A, true);
    pad.setButton(Controller::START, true);

    // Strobe ON
    pad.write(0x01);
    assert(pad.read() == 1); // Button A is bit 7 (first bit read out in NES standard sequence)

    // Strobe OFF to begin shifting
    pad.write(0x00);
    assert(pad.read() == 1); // A (bit 7)
    assert(pad.read() == 0); // B (bit 6)
    assert(pad.read() == 0); // SELECT (bit 5)
    assert(pad.read() == 1); // START (bit 4)
    assert(pad.read() == 0); // UP (bit 3)
    assert(pad.read() == 0); // DOWN (bit 2)
    assert(pad.read() == 0); // LEFT (bit 1)
    assert(pad.read() == 0); // RIGHT (bit 0)

    std::cout << "testController_StrobeAndShift passed!" << std::endl;
}

int main() {
    testController_StrobeAndShift();
    return 0;
}
