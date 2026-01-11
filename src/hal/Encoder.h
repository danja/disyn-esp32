#pragma once

#include <cstdint>

namespace disyn::hal {

class Encoder {
public:
    void begin(int pinClk, int pinDt, int pinSw);
    void update();
    int32_t position() const;
    bool pressed();
    bool isDown() const;

private:
    int pinClk_ = -1;
    int pinDt_ = -1;
    int pinSw_ = -1;
    int32_t position_ = 0;
    int lastClk_ = 0;
    int lastSw_ = 1;
    bool pressedEvent_ = false;
    bool down_ = false;
};

} // namespace disyn::hal
