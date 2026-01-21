#include "hal/Display.h"

#include <Arduino.h>
#include <Wire.h>

#include "PinConfig.h"

namespace disyn::hal {

bool Display::begin()
{
    Wire.begin(kPinI2cSda, kPinI2cScl);
    if (!display_.begin(0x3C, true))
    {
        return false;
    }
    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SH110X_WHITE);
    display_.display();
    return true;
}

void Display::clear()
{
    display_.clearDisplay();
}

void Display::display()
{
    display_.display();
}

void Display::setTextSize(uint8_t size)
{
    display_.setTextSize(size);
}

void Display::setTextColor(uint16_t color)
{
    display_.setTextColor(color);
}

void Display::setCursor(int16_t x, int16_t y)
{
    display_.setCursor(x, y);
}

size_t Display::print(const char *text)
{
    return display_.print(text);
}

size_t Display::println(const char *text)
{
    return display_.println(text);
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    display_.drawPixel(x, y, color);
}

} // namespace disyn::hal
