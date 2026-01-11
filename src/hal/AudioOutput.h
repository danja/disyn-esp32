#pragma once

#include <cstddef>
#include <cstdint>

namespace disyn::hal {

class AudioOutput {
public:
    bool begin(int sampleRate, int bufferLength);
    bool write(const uint16_t *buffer, size_t length, size_t *bytesWritten);
};

} // namespace disyn::hal
