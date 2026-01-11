#pragma once

#include <algorithm>
#include <cmath>

namespace flues::disyn {

class EnvelopeModule {
public:
    explicit EnvelopeModule(float sampleRate = 44100.0f)
        : sampleRate(sampleRate),
          attackTime(0.2f),
          releaseTime(0.4f),
          attackNorm(0.5f),
          releaseNorm(0.5f),
          envelope(0.0f),
          gate(false),
          isActive(false) {}

    void setAttack(float value) {
        const float minTime = 0.001f;
        const float maxTime = 1.0f;
        attackNorm = std::clamp(value, 0.0f, 1.0f);
        attackTime = minTime * std::pow(maxTime / minTime, attackNorm);
    }

    void setRelease(float value) {
        const float minTime = 0.01f;
        const float maxTime = 3.0f;
        releaseNorm = std::clamp(value, 0.0f, 1.0f);
        releaseTime = minTime * std::pow(maxTime / minTime, releaseNorm);
    }

    void setGate(bool gateState) {
        gate = gateState;
        if (gate) {
            isActive = true;
        }
    }

    float process() {
        if (attackNorm <= 0.0f && releaseNorm <= 0.0f) {
            envelope = 1.0f;
            isActive = true;
            return envelope;
        }

        if (gate) {
            const float attackRate = 1.0f / std::max(attackTime * sampleRate, 1.0f);
            envelope += attackRate;
            if (envelope > 1.0f) {
                envelope = 1.0f;
            }
        } else {
            const float releaseRate = 1.0f / std::max(releaseTime * sampleRate, 1.0f);
            envelope -= releaseRate;
            if (envelope < 0.0f) {
                envelope = 0.0f;
                isActive = false;
            }
        }
        return envelope;
    }

    bool isPlaying() const {
        return isActive;
    }

    void reset() {
        envelope = 0.0f;
        isActive = true;
    }

private:
    float sampleRate;
    float attackTime;
    float releaseTime;
    float attackNorm;
    float releaseNorm;
    float envelope;
    bool gate;
    bool isActive;
};

} // namespace flues::disyn
