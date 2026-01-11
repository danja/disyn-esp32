#pragma once

#include <cstdint>

namespace disyn::hal {

class Dac {
public:
    void begin();
    void write(uint8_t left, uint8_t right);
};

} // namespace disyn::hal
