#pragma once

#include <cstdint>

namespace disyn::hal {

class Adc {
public:
    void begin();
    uint16_t readCv0() const;
    uint16_t readCv1() const;
    uint16_t readCv2() const;
    uint16_t readPot0() const;
    uint16_t readPot1() const;
    uint16_t readPot2() const;
};

} // namespace disyn::hal
