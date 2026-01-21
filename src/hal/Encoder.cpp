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

    int clk = digitalRead(pinClk_);
    int dt = digitalRead(pinDt_);
    lastState_ = static_cast<uint8_t>(((clk != 0) ? 1 : 0) << 1 | ((dt != 0) ? 1 : 0));
    lastSwRaw_ = digitalRead(pinSw_);
    stableSw_ = lastSwRaw_;
    lastMoveUs_ = micros();
    lastSwChangeMs_ = millis();
}

void Encoder::update()
{
    int clk = digitalRead(pinClk_);
    int dt = digitalRead(pinDt_);
    uint8_t state = static_cast<uint8_t>(((clk != 0) ? 1 : 0) << 1 | ((dt != 0) ? 1 : 0));
    if (state != lastState_)
    {
        uint32_t now = micros();
        constexpr uint32_t kRotationDebounceUs = 1500;
        if (now - lastMoveUs_ >= kRotationDebounceUs)
        {
            static constexpr int8_t kEncoderTable[16] = {
                0, -1,  1,  0,
                1,  0,  0, -1,
               -1,  0,  0,  1,
                0,  1, -1,  0
            };
            uint8_t index = static_cast<uint8_t>((lastState_ << 2) | state);
            int8_t delta = kEncoderTable[index];
            if (delta != 0)
            {
                stepAccum_ += delta;
                if (stepAccum_ >= 2 || stepAccum_ <= -2)
                {
                    position_ += stepAccum_ / 2;
                    stepAccum_ = 0;
                    lastMoveUs_ = now;
                }
            }
        }
        lastState_ = state;
    }

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
