#include "hal/Gate.h"

#include <Arduino.h>

namespace disyn::hal {

void Gate::begin(int pinIn, int pinOut)
{
    pinIn_ = pinIn;
    pinOut_ = pinOut;

    // read() treats LOW as "gate asserted" (inverting input buffer). Without a
    // pull-up an unpatched jack floats and reads as a stuck/random trigger, so
    // pull high to make the idle state gate-off.
    pinMode(pinIn_, INPUT_PULLUP);
    pinMode(pinOut_, OUTPUT);
    digitalWrite(pinOut_, HIGH); // idle high = silent, per docs/manual.md
}

bool Gate::read() const
{
    return digitalRead(pinIn_) == LOW;
}

void Gate::write(bool high)
{
    digitalWrite(pinOut_, high ? HIGH : LOW);
}

} // namespace disyn::hal
