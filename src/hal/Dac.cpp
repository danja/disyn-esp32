#include "hal/Dac.h"

#include <Arduino.h>

#include "PinConfig.h"

namespace disyn::hal {

void Dac::begin()
{
    pinMode(kPinDac1, OUTPUT);
    pinMode(kPinDac2, OUTPUT);
}

void Dac::write(uint8_t left, uint8_t right)
{
    dacWrite(kPinDac1, left);
    dacWrite(kPinDac2, right);
}

} // namespace disyn::hal
