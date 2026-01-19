#pragma once

#include <Adafruit_SH110X.h>

namespace disyn::hal {

class Display {
public:
    bool begin();
    void clear();
    void display();
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    size_t print(const char *text);
    size_t println(const char *text);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

private:
    Adafruit_SH1106G display_{128, 64, &Wire};
};

} // namespace disyn::hal
