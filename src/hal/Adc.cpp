#include "hal/Adc.h"

#include <Arduino.h>
#include <driver/adc.h>

#include "PinConfig.h"

namespace disyn::hal
{

    namespace
    {
        constexpr adc1_channel_t kCv0Channel = ADC1_CHANNEL_0; // GPIO36
        constexpr adc1_channel_t kCv1Channel = ADC1_CHANNEL_3; // GPIO39
        bool sUseIdf = false;

        uint16_t readAdc1(adc1_channel_t channel)
        {
            const int raw = adc1_get_raw(channel);
            return raw < 0 ? 0u : static_cast<uint16_t>(raw);
        }
    } // namespace

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

        // ESP-IDF ADC1 setup for GPIO36/39 to avoid Arduino attach issues on these pins.
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(kCv0Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kCv1Channel, ADC_ATTEN_DB_11);
        sUseIdf = true;
    }

    uint16_t Adc::readCv0() const
    {
        return sUseIdf ? readAdc1(kCv0Channel) : analogRead(kPinCv0);
    }

    uint16_t Adc::readCv1() const
    {
        return sUseIdf ? readAdc1(kCv1Channel) : analogRead(kPinCv1);
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
