#pragma once

#include <cstdint>

namespace disyn {

struct AdcCalibration {
    uint16_t minValue;
    uint16_t maxValue;
    bool invert;
};

constexpr AdcCalibration kCv0Cal{0, 4095, false};
constexpr AdcCalibration kCv1Cal{0, 4095, false};
constexpr AdcCalibration kCv2Cal{0, 4095, false};
constexpr AdcCalibration kPot0Cal{0, 4095, false};
constexpr AdcCalibration kPot1Cal{0, 4095, false};
constexpr AdcCalibration kPot2Cal{0, 4095, false};

inline float NormalizeAdc(uint16_t value, const AdcCalibration &cal)
{
    if (cal.maxValue <= cal.minValue)
    {
        return 0.0f;
    }
    float normalized = static_cast<float>(value - cal.minValue) /
        static_cast<float>(cal.maxValue - cal.minValue);
    if (normalized < 0.0f)
    {
        normalized = 0.0f;
    }
    if (normalized > 1.0f)
    {
        normalized = 1.0f;
    }
    if (cal.invert)
    {
        normalized = 1.0f - normalized;
    }
    return normalized;
}

} // namespace disyn
