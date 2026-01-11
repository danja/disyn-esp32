#include "hal/Gate.h"

#include <Arduino.h>

namespace disyn::hal {

void Gate::begin(int pinIn, int pinOut)
{
    pinIn_ = pinIn;
    pinOut_ = pinOut;

    pinMode(pinIn_, INPUT);
    pinMode(pinOut_, OUTPUT);
}

bool Gate::read() const
{
    return digitalRead(pinIn_) == HIGH;
}

void Gate::write(bool high)
{
    digitalWrite(pinOut_, high ? HIGH : LOW);
}

} // namespace disyn::hal
