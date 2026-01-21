#include "hal/Adc.h"

#include <Arduino.h>

#include "PinConfig.h"

namespace disyn::hal
{

    void Adc::begin()
    {
        analogReadResolution(12);
        // Set pins as inputs
        pinMode(kPinCv0, INPUT);
        pinMode(kPinCv1, INPUT);
        pinMode(kPinCv2, INPUT);
        pinMode(kPinPot0, INPUT);
        pinMode(kPinPot1, INPUT);
        pinMode(kPinPot2, INPUT);

        // Ensure pins are attached to ADC and have expected attenuation.
        adcAttachPin(kPinCv0);
        adcAttachPin(kPinCv1);
        adcAttachPin(kPinCv2);
        adcAttachPin(kPinPot0);
        adcAttachPin(kPinPot1);
        adcAttachPin(kPinPot2);

        analogSetPinAttenuation(kPinCv0, ADC_11db);
        analogSetPinAttenuation(kPinCv1, ADC_11db);
        analogSetPinAttenuation(kPinCv2, ADC_11db);
        analogSetPinAttenuation(kPinPot0, ADC_11db);
        analogSetPinAttenuation(kPinPot1, ADC_11db);
        analogSetPinAttenuation(kPinPot2, ADC_11db);
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
