#include "hal/Adc.h"

#include <Arduino.h>

#include "PinConfig.h"

namespace disyn::hal {

void Adc::begin()
{
    analogReadResolution(12);
}

uint16_t Adc::readCv0() const
{
    return analogRead(kPinCv0);
}

uint16_t Adc::readCv1() const
{
    return analogRead(kPinCv1);
}

uint16_t Adc::readCv2() const
{
    return analogRead(kPinCv2);
}

uint16_t Adc::readPot0() const
{
    return analogRead(kPinPot0);
}

uint16_t Adc::readPot1() const
{
    return analogRead(kPinPot1);
}

uint16_t Adc::readPot2() const
{
    return analogRead(kPinPot2);
}

} // namespace disyn::hal
