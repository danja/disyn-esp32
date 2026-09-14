#include "hal/Adc.h"

#include <Arduino.h>
#include <driver/adc.h>

#include "PinConfig.h"

namespace disyn::hal
{

    namespace
    {
        // All six inputs live on ADC1. Do not move any of them to an ADC2 pin:
        // the I2S built-in DAC (GPIO25/26 = ADC2_CHANNEL_8/9) drives the same
        // RTC/SAR block continuously, so adc2_get_raw() cannot win arbitration
        // and reads are dead or intermittent.
        constexpr adc1_channel_t kCv0Channel = ADC1_CHANNEL_0;  // GPIO36
        constexpr adc1_channel_t kCv1Channel = ADC1_CHANNEL_3;  // GPIO39
        constexpr adc1_channel_t kCv2Channel = ADC1_CHANNEL_4;  // GPIO32
        constexpr adc1_channel_t kPot0Channel = ADC1_CHANNEL_5; // GPIO33
        constexpr adc1_channel_t kPot1Channel = ADC1_CHANNEL_6; // GPIO34
        constexpr adc1_channel_t kPot2Channel = ADC1_CHANNEL_7; // GPIO35

        uint16_t readAdc1(adc1_channel_t channel)
        {
            const int raw = adc1_get_raw(channel);
            return raw < 0 ? 0u : static_cast<uint16_t>(raw);
        }
    } // namespace

    void Adc::begin()
    {
        // Use the ESP-IDF API for every channel. Arduino's analogRead() re-runs
        // __adcAttachPin() (pinMode(ANALOG) + adc1_config_channel_atten) on every
        // single call, which reconfigures shared ADC1 state underneath any direct
        // adc1_get_raw() reads. Mixing the two APIs is what made this flaky.
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(kCv0Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kCv1Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kCv2Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kPot0Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kPot1Channel, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(kPot2Channel, ADC_ATTEN_DB_11);
    }

    uint16_t Adc::readCv0() const
    {
        return readAdc1(kCv0Channel);
    }

    uint16_t Adc::readCv1() const
    {
        return readAdc1(kCv1Channel);
    }

    uint16_t Adc::readCv2() const
    {
        return readAdc1(kCv2Channel);
    }

    uint16_t Adc::readPot0() const
    {
        return readAdc1(kPot0Channel);
    }

    uint16_t Adc::readPot1() const
    {
        return readAdc1(kPot1Channel);
    }

    uint16_t Adc::readPot2() const
    {
        return readAdc1(kPot2Channel);
    }

} // namespace disyn::hal
