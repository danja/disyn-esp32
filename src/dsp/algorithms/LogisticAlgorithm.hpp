#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class LogisticAlgorithm {
public:
    explicit LogisticAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          lastPhase(0.0f),
          x(0.37f),
          smoothed(0.0f) {}

    void reset() {
        phase = 0.0f;
        lastPhase = 0.0f;
        x = 0.37f;
        smoothed = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param3;
        const float smoothing = std::clamp(param1, 0.0f, 1.0f);
        const float slewCoeff = 0.02f + (1.0f - smoothing) * 0.18f;

        phase = stepPhase(phase, pitch, sampleRate);
        if (phase < lastPhase) {
            const float r = 3.9f;
            x = r * x * (1.0f - x);
            x = std::clamp(x, 0.0001f, 0.9999f);
        }
        lastPhase = phase;

        const float raw = x * 2.0f - 1.0f;
        const float output = slewLimit(raw, smoothed, slewCoeff);
        return {output, raw};
    }

private:
    float sampleRate;
    float phase;
    float lastPhase;
    float x;
    float smoothed;
};

} // namespace flues::disyn
