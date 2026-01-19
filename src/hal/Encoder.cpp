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
    lastSwRaw_ = digitalRead(pinSw_);
    stableSw_ = lastSwRaw_;
    lastMoveUs_ = micros();
    lastSwChangeMs_ = millis();
}

void Encoder::update()
{
    int clk = digitalRead(pinClk_);
    if (clk == LOW && lastClk_ == HIGH)
    {
        uint32_t now = micros();
        constexpr uint32_t kRotationDebounceUs = 1000;
        if (now - lastMoveUs_ >= kRotationDebounceUs)
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
            lastMoveUs_ = now;
        }
    }
    lastClk_ = clk;

    int swRaw = digitalRead(pinSw_);
    if (swRaw != lastSwRaw_)
    {
        lastSwRaw_ = swRaw;
        lastSwChangeMs_ = millis();
    }
    else
    {
        constexpr uint32_t kButtonDebounceMs = 20;
        if (millis() - lastSwChangeMs_ >= kButtonDebounceMs && swRaw != stableSw_)
        {
            stableSw_ = swRaw;
            if (stableSw_ == LOW)
            {
                pressedEvent_ = true;
            }
        }
    }
    down_ = (stableSw_ == LOW);
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
