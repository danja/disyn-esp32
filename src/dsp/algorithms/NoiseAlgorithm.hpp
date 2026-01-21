#pragma once

#include <cstdint>

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class NoiseAlgorithm {
public:
    explicit NoiseAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          lastPhase(0.0f),
          noiseValue(0.0f),
          smoothed(0.0f),
          rngState(0x6d2b79f5u) {}

    void reset() {
        phase = 0.0f;
        lastPhase = 0.0f;
        noiseValue = 0.0f;
        smoothed = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param3;
        const float smoothing = std::clamp(param1, 0.0f, 1.0f);
        const float slewCoeff = 0.02f + (1.0f - smoothing) * 0.98f;

        phase = stepPhase(phase, pitch, sampleRate);
        if (phase < lastPhase) {
            noiseValue = nextNoise();
        }
        lastPhase = phase;

        const float output = slewLimit(noiseValue, smoothed, slewCoeff);
        return {output, noiseValue};
    }

private:
    float nextNoise() {
        rngState = rngState * 1664525u + 1013904223u;
        const uint32_t sample = (rngState >> 8) & 0xFFFFFFu;
        return static_cast<float>(sample) / 8388607.5f - 1.0f;
    }

    float sampleRate;
    float phase;
    float lastPhase;
    float noiseValue;
    float smoothed;
    uint32_t rngState;
};

} // namespace flues::disyn
