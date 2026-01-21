#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class RampAlgorithm {
public:
    explicit RampAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f) {}

    void reset() {
        phase = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param3;
        const int steps = 1 + static_cast<int>(std::round(std::clamp(param1, 0.0f, 1.0f) * 63.0f));

        phase = stepPhase(phase, pitch, sampleRate);
        float value = phase * 2.0f - 1.0f;
        value = quantizeBipolar(value, steps);
        return {value, value};
    }

private:
    float sampleRate;
    float phase;
};

} // namespace flues::disyn
