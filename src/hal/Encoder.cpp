#include "hal/Encoder.h"

#include <Arduino.h>

namespace disyn::hal {

void Encoder::begin(int pinClk, int pinDt, int pinSw)
{
    pinClk_ = pinClk;
    pinDt_ = pinDt;
    pinSw_ = pinSw;

    pinMode(pinClk_, INPUT);
    pinMode(pinDt_, INPUT);
    pinMode(pinSw_, INPUT_PULLUP);

    lastClk_ = digitalRead(pinClk_);
    lastSw_ = digitalRead(pinSw_);
}

void Encoder::update()
{
    int clk = digitalRead(pinClk_);
    if (clk == LOW && lastClk_ == HIGH)
    {
        int dt = digitalRead(pinDt_);
        if (dt == HIGH)
        {
            ++position_;
        }
        else
        {
            --position_;
        }
    }
    lastClk_ = clk;

    int sw = digitalRead(pinSw_);
    if (lastSw_ == HIGH && sw == LOW)
    {
        pressedEvent_ = true;
    }
    lastSw_ = sw;
    down_ = (sw == LOW);
}

int32_t Encoder::position() const
{
    return position_;
}

bool Encoder::pressed()
{
    bool wasPressed = pressedEvent_;
    pressedEvent_ = false;
    return wasPressed;
}

bool Encoder::isDown() const
{
    return down_;
}

} // namespace disyn::hal
