#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class PulseAlgorithm {
public:
    explicit PulseAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f) {}

    void reset() {
        phase = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param3;
        const float width = 0.05f + std::clamp(param1, 0.0f, 1.0f) * 0.9f;

        phase = stepPhase(phase, pitch, sampleRate);
        const float value = (phase < width) ? 1.0f : -1.0f;
        return {value, -value};
    }

private:
    float sampleRate;
    float phase;
};

} // namespace flues::disyn
