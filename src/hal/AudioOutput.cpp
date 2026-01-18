#include "hal/AudioOutput.h"

#include <driver/i2s.h>

namespace disyn::hal {

bool AudioOutput::begin(int sampleRate, int bufferLength)
{
    i2s_config_t config{};
    config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
    config.sample_rate = sampleRate;
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.communication_format = I2S_COMM_FORMAT_STAND_MSB;
    config.intr_alloc_flags = 0;
    config.dma_buf_count = 8;
    config.dma_buf_len = bufferLength;
    config.use_apll = false;
    config.tx_desc_auto_clear = true;
    config.fixed_mclk = 0;

    if (i2s_driver_install(I2S_NUM_0, &config, 0, nullptr) != ESP_OK)
    {
        return false;
    }

    if (i2s_set_pin(I2S_NUM_0, nullptr) != ESP_OK)
    {
        return false;
    }

    if (i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN) != ESP_OK)
    {
        return false;
    }

    return true;
}

bool AudioOutput::write(const uint16_t *buffer, size_t length, size_t *bytesWritten)
{
    if (i2s_write(I2S_NUM_0, buffer, length, bytesWritten, portMAX_DELAY) != ESP_OK)
    {
        return false;
    }
    return true;
}

} // namespace disyn::hal
