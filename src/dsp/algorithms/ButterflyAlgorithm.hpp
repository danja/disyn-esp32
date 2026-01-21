#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class ButterflyAlgorithm {
public:
    explicit ButterflyAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          x(0.1f),
          y(0.0f),
          z(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        x = 0.1f;
        y = 0.0f;
        z = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param3;
        const float smoothing = std::clamp(param1, 0.0f, 1.0f);
        const float slewCoeff = 0.01f + (1.0f - smoothing) * 0.19f;
        const float dt = std::clamp(pitch / sampleRate, 0.0001f, 0.05f);

        const float sigma = 10.0f;
        const float rho = 28.0f;
        const float beta = 2.6666667f;

        const float dx = sigma * (y - x);
        const float dy = x * (rho - z) - y;
        const float dz = x * y - beta * z;

        x += dx * dt;
        y += dy * dt;
        z += dz * dt;

        const float rawPrimary = softClip(x * 0.05f);
        const float rawSecondary = softClip(y * 0.05f);
        const float smoothedPrimary = slewLimit(rawPrimary, outPrimary, slewCoeff);
        const float smoothedSecondary = slewLimit(rawSecondary, outSecondary, slewCoeff);
        return {smoothedPrimary, smoothedSecondary};
    }

private:
    float sampleRate;
    float x;
    float y;
    float z;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
