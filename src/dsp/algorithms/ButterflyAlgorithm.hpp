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
        const float sigma = 10.0f;
        const float rho = 28.0f;
        const float beta = 2.6666667f;

        // Forward Euler on the Lorenz system diverges above dt ~= 0.023, and the
        // old clamp allowed 0.05 -- so every pitch above ~970Hz blew the state up
        // to Inf/NaN within a few dozen samples. Sub-step instead, so pitch still
        // tracks but each step stays inside the stable region.
        constexpr float kMaxStableDt = 0.02f;
        constexpr int kMaxSteps = 8;
        const float dtWanted = std::clamp(pitch / sampleRate, 0.0001f, 0.1f);
        int steps = 1 + static_cast<int>(dtWanted / kMaxStableDt);
        if (steps > kMaxSteps) {
            steps = kMaxSteps;
        }
        const float dt = dtWanted / static_cast<float>(steps);

        for (int step = 0; step < steps; ++step) {
            const float dx = sigma * (y - x);
            const float dy = x * (rho - z) - y;
            const float dz = x * y - beta * z;

            x += dx * dt;
            y += dy * dt;
            z += dz * dt;
        }

        // Bounded-range guard. Plain comparisons, which are also false for NaN,
        // so this catches divergence without std::isfinite -- docs/danger-zones.md
        // records isfinite/numeric_limits guards crashing on hardware. Same idiom
        // already used in WavefolderModule.
        if (!(x > -1000.0f && x < 1000.0f) ||
            !(y > -1000.0f && y < 1000.0f) ||
            !(z > -1000.0f && z < 1000.0f)) {
            x = 0.1f;
            y = 0.0f;
            z = 0.0f;
        }

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
