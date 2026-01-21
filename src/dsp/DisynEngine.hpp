#pragma once

#include <cmath>
#include <algorithm>

#include "algorithms/AlgorithmOutput.hpp"
#include "algorithms/AlgorithmTypes.hpp"
#include "modules/OscillatorModule.hpp"
#include "modules/WavefolderModule.hpp"
#include "modules/EnvelopeModule.hpp"
#include "modules/ReverbModule.hpp"

namespace flues::disyn
{

    class DisynEngine
    {
    public:
        explicit DisynEngine(float sampleRate = 44100.0f)
            : sampleRate(sampleRate),
              oscillator(sampleRate),
              wavefolder(),
              envelope(sampleRate),
              reverbLeft(sampleRate),
              reverbRight(sampleRate),
              frequency(440.0f),
              algorithmType(AlgorithmType::TANH_SQUARE),
              param1(0.55f), // Default drive for tanh square
              param2(0.5f),  // Default trim for tanh square
              param3(0.5f),
              wavefoldAmount(0.0f),
              masterGain(0.8f),
              velocity(1.0f),
              gate(false),
              isPlaying(false)
        {
        }

        void noteOn(float freq, float vel = 1.0f)
        {
            frequency = freq;
            velocity = std::clamp(vel, 0.0f, 1.0f);
            gate = true;
            isPlaying = true;

            oscillator.reset();
            envelope.reset();
            reverbLeft.reset();
            reverbRight.reset();

            envelope.setGate(true);
        }

        void noteOff()
        {
            gate = false;
            envelope.setGate(false);
        }

        void setFrequency(float freq)
        {
            frequency = std::max(freq, 0.0f);
        }

        AlgorithmOutput process()
        {
            if (!isPlaying)
            {
                return {0.0f, 0.0f};
            }

            // Generate oscillator sample
            const AlgorithmOutput oscOutput = oscillator.process(algorithmType, frequency, param1, param2, param3);
            float primary = oscOutput.primary;
            float secondary = oscOutput.secondary;
            const float foldGain = getAlgorithmFoldGain(algorithmType);
            const float outputGain = getAlgorithmOutputGain(algorithmType);
            const float preFoldPrimary = std::tanh(primary * foldGain);
            const float preFoldSecondary = std::tanh(secondary * foldGain);
            const float foldedPrimary = std::tanh(wavefolder.process(preFoldPrimary, wavefoldAmount));
            const float foldedSecondary = std::tanh(wavefolder.process(preFoldSecondary, wavefoldAmount));

            // Apply envelope
            const float env = envelope.process();

            // Apply velocity and master gain
            const float leftSample = foldedPrimary * env * velocity * masterGain * outputGain;
            const float rightSample = foldedSecondary * env * velocity * masterGain * outputGain;

            // Apply reverb
            const float left = reverbLeft.process(leftSample);
            const float right = reverbRight.process(rightSample);

            // Voice tail detection - stop if envelope is silent
            if (!envelope.isPlaying() &&
                std::max(std::abs(left), std::abs(right)) < 1e-5f)
            {
                isPlaying = false;
            }

            return {left, right};
        }

        // Parameter setters
        void setAlgorithm(int type)
        {
            if (type >= 0 && type <= 18)
            {
                algorithmType = static_cast<AlgorithmType>(type);
            }
        }

        void setParam1(float value)
        {
            param1 = std::clamp(value, 0.0f, 1.0f);
        }

        void setParam2(float value)
        {
            param2 = std::clamp(value, 0.0f, 1.0f);
        }

        void setParam3(float value)
        {
            param3 = std::clamp(value, 0.0f, 1.0f);
        }

        void setWavefoldAmount(float value)
        {
            wavefoldAmount = std::clamp(value, 0.0f, 1.0f);
        }

        void setAttack(float value)
        {
            envelope.setAttack(value);
        }

        void setRelease(float value)
        {
            envelope.setRelease(value);
        }

        void setReverbSize(float value)
        {
            reverbLeft.setSize(value);
            reverbRight.setSize(value);
        }

        void setReverbLevel(float value)
        {
            reverbLeft.setLevel(value);
            reverbRight.setLevel(value);
        }

        void setMasterGain(float value)
        {
            masterGain = std::clamp(value, 0.0f, 1.0f);
        }

        bool getIsPlaying() const
        {
            return isPlaying;
        }

    private:
        static float getAlgorithmFoldGain(AlgorithmType type)
        {
            switch (type)
            {
            case AlgorithmType::DIRICHLET_PULSE:
                return 0.901067f;
            case AlgorithmType::DSF_SINGLE:
                return 0.2450f;
            case AlgorithmType::DSF_DOUBLE:
                return 0.03480f;
            case AlgorithmType::TANH_SQUARE:
                return 0.10000f;
            case AlgorithmType::TANH_SAW:
                return 0.2000f;
            case AlgorithmType::PAF:
                return 0.01000f;
            case AlgorithmType::MOD_FM:
                return 0.01000f;
            case AlgorithmType::COMBINATION_1_HYBRID_FORMANT:
                return 0.0500f;
            case AlgorithmType::COMBINATION_2_CASCADED:
                return 0.01000f;
            case AlgorithmType::COMBINATION_3_PARALLEL_BANK:
                return 0.01000f;
            case AlgorithmType::COMBINATION_4_FEEDBACK:
                return 0.1000f;
            case AlgorithmType::COMBINATION_5_MORPHING:
                return 0.01000f;
            case AlgorithmType::COMBINATION_6_INHARMONIC:
                return 1.0000f;
            case AlgorithmType::COMBINATION_7_ADAPTIVE_FILTER:
                return 0.00040f;
            case AlgorithmType::NOVEL_1_MULTISTAGE:
                return 0.00355f;
            case AlgorithmType::NOVEL_2_FREQ_ASYMMETRY:
                return 0.000699f;
            case AlgorithmType::NOVEL_3_CROSS_MOD:
                return 0.01000f;
            case AlgorithmType::NOVEL_4_TAYLOR:
                return 1.0000f;
            case AlgorithmType::TRAJECTORY:
                return 1.0000f;
            default:
                return 1.0f;
            }
        }

        static float getAlgorithmOutputGain(AlgorithmType type)
        {
            switch (type)
            {
            case AlgorithmType::DIRICHLET_PULSE:
                return 0.4721f;
            case AlgorithmType::DSF_SINGLE:
                return 1.0000f;
            case AlgorithmType::DSF_DOUBLE:
                return 1.0000f;
            case AlgorithmType::TANH_SQUARE:
                return 0.20f;
            case AlgorithmType::TANH_SAW:
                return 0.4991f;
            case AlgorithmType::PAF:
                return 0.80f;
            case AlgorithmType::MOD_FM:
                return 0.60f;
            case AlgorithmType::COMBINATION_1_HYBRID_FORMANT:
                return 1.0f;
            case AlgorithmType::COMBINATION_2_CASCADED:
                return 0.40f;
            case AlgorithmType::COMBINATION_3_PARALLEL_BANK:
                return 0.60f;
            case AlgorithmType::COMBINATION_4_FEEDBACK:
                return 0.8208f;
            case AlgorithmType::COMBINATION_5_MORPHING:
                return 0.50f;
            case AlgorithmType::COMBINATION_6_INHARMONIC:
                return 0.7967f;
            case AlgorithmType::COMBINATION_7_ADAPTIVE_FILTER:
                return 0.500f;
            case AlgorithmType::NOVEL_1_MULTISTAGE:
                return 0.40f;
            case AlgorithmType::NOVEL_2_FREQ_ASYMMETRY:
                return 0.50f;
            case AlgorithmType::NOVEL_3_CROSS_MOD:
                return 0.9917f;
            case AlgorithmType::NOVEL_4_TAYLOR:
                return 0.4501f;
            case AlgorithmType::TRAJECTORY:
                return 0.6886f;
            default:
                return 1.0f;
            }
        }
        float sampleRate;
        OscillatorModule oscillator;
        WavefolderModule wavefolder;
        EnvelopeModule envelope;
        ReverbModule reverbLeft;
        ReverbModule reverbRight;

        float frequency;
        AlgorithmType algorithmType;
        float param1;
        float param2;
        float param3;
        float wavefoldAmount;
        float masterGain;
        float velocity;
        bool gate;
        bool isPlaying;
    };

} // namespace flues::disyn
