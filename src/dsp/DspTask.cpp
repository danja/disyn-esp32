#include "dsp/DspTask.h"

#include <cstring>
#include <cmath>

#include "IntercoreQueue.h"
#include "Parameters.h"
#include "PinConfig.h"
#include "Config.h"
#include "AlgorithmInfo.h"
#include "dsp/DisynEngine.hpp"
#include "hal/AudioOutput.h"
#include "hal/Gate.h"

namespace disyn::dsp {

static disyn::hal::Gate gate;
static disyn::hal::AudioOutput audioOut;
static disyn::Parameters params;
static float effectiveParam1 = 0.0f;
static float effectiveParam2 = 0.0f;
static flues::disyn::DisynEngine engine{kSampleRate};
static bool lastGate = false;
static float smoothPitch = 0.0f;
static float testPhase = 0.0f;
static float hardwarePhase = 0.0f;
static uint8_t lastAlgorithm = 0;
constexpr int kAudioBlockSize = 64;
static uint16_t audioBlock[kAudioBlockSize * 2] = {};
static uint32_t underrunCount = 0;
static float outputGain = 0.8f;
static bool audioOk = true;

static float softClip(float value)
{
    return std::tanh(value);
}

static float clamp01(float value)
{
    if (value < 0.0f)
    {
        return 0.0f;
    }
    if (value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

static bool isUnusedParam(const char *label)
{
    return label != nullptr && std::strcmp(label, "Unused") == 0;
}

static float clamp(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

static float smoothValue(float current, float target, float alpha)
{
    return current + alpha * (target - current);
}

static uint16_t sampleToDac(float sample)
{
    float normalized = sample * 0.5f + 0.5f;
    normalized = clamp01(normalized);
    return static_cast<uint16_t>(normalized * 255.0f) << 8;
}

static void Init()
{
    gate.begin(kPinGateIn, kPinGateOut);
    if (!audioOut.begin(kSampleRate, kAudioBlockSize))
    {
        ++underrunCount;
        audioOk = false;
    }
}

static void Tick()
{
    // TODO: run audio processing at the configured sample rate.
    if (disyn::gParamQueue != nullptr)
    {
        disyn::ParamMessage message{};
        if (xQueueReceive(disyn::gParamQueue, &message, 0) == pdTRUE)
        {
            params = message.params;
        }
    }

    bool gateHigh = gate.read();

    const auto &algoInfo = disyn::GetAlgorithmInfo(params.algorithm);
    if (isUnusedParam(algoInfo.param1.label))
    {
        effectiveParam1 = 0.5f;
    }
    else
    {
    effectiveParam1 = clamp01(params.param1 + (params.cv0 - 0.5f) * kParamModAmount + (params.pot0 - 0.5f) * kParamModAmount);
    }

    if (isUnusedParam(algoInfo.param2.label))
    {
        effectiveParam2 = 0.5f;
    }
    else
    {
    effectiveParam2 = clamp01(params.param2 + (params.cv1 - 0.5f) * kParamModAmount + (params.pot1 - 0.5f) * kParamModAmount);
    }


    float pitchCv = clamp01(1.0f - params.cv2);
    float pitchPot = clamp01(1.0f - params.pot2);
    float pitchControl = clamp01(pitchCv * kPitchCvMix + pitchPot * kPitchPotMix);
    constexpr float kPitchAlpha = 0.5f;
    smoothPitch = smoothValue(smoothPitch, pitchControl, kPitchAlpha);
    float frequency = 55.0f + smoothPitch * (880.0f - 55.0f);

    bool forceContinuous = params.attack <= 0.0f && params.decay <= 0.0f;
    bool engineGate = gateHigh || forceContinuous;
    bool isTest = params.algorithm == disyn::kTestAlgorithmIndex;
    bool isHardware = params.algorithm == disyn::kHardwareAlgorithmIndex;

    if (params.algorithm != lastAlgorithm)
    {
        lastGate = false;
        testPhase = 0.0f;
        lastAlgorithm = params.algorithm;
    }

    float reverbSize = clamp01(params.reverbSize + (params.cv2 - 0.5f) * kReverbSizeCvAmount +
                               (params.pot2 - 0.5f) * kReverbSizePotAmount);
    float reverbLevel = clamp01(params.reverbLevel + (params.cv2 - 0.5f) * kReverbLevelCvAmount +
                                (params.pot2 - 0.5f) * kReverbLevelPotAmount);
    float masterGain = clamp01(params.masterGain + (params.cv2 - 0.5f) * kMasterCvAmount +
                               (params.pot2 - 0.5f) * kMasterPotAmount);

    if (!isTest)
    {
        engine.setAlgorithm(params.algorithm);
        engine.setParam1(effectiveParam1);
        engine.setParam2(effectiveParam2);
        engine.setAttack(params.attack);
        engine.setRelease(params.decay);
        engine.setReverbSize(reverbSize);
        engine.setReverbLevel(reverbLevel);
        engine.setMasterGain(masterGain);
        engine.setFrequency(frequency);

        if (engineGate && !lastGate)
        {
            engine.noteOn(frequency, 1.0f);
        }
        else if (!engineGate && lastGate)
        {
            engine.noteOff();
        }
    }
    lastGate = engineGate;

    for (int i = 0; i < kAudioBlockSize; ++i)
    {
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        outputGain = masterGain;

        if (isHardware)
        {
            constexpr float kHardwareFreq = 440.0f;
            float phaseStep = kHardwareFreq / static_cast<float>(kSampleRate);
            hardwarePhase += phaseStep;
            if (hardwarePhase >= 1.0f)
            {
                hardwarePhase -= 1.0f;
            }
            float tone = hardwarePhase < 0.5f ? 1.0f : -1.0f;
            leftSample = softClip(tone * outputGain);
            rightSample = leftSample;
        }
        else if (isTest)
        {
            const auto &info = disyn::GetAlgorithmInfo(disyn::kTestAlgorithmIndex);
            float testFreq = disyn::MapNormalized(info.param1, effectiveParam1);
            float testLevel = disyn::MapNormalized(info.param2, effectiveParam2);
            float phaseStep = testFreq / static_cast<float>(kSampleRate);
            testPhase += phaseStep;
            if (testPhase >= 1.0f)
            {
                testPhase -= 1.0f;
            }
            constexpr float kTwoPi = 6.283185307179586f;
            float tone = std::sin(testPhase * kTwoPi);
            float gateLevel = engineGate ? 1.0f : 0.0f;
            float sampleValue = tone * testLevel * gateLevel;
            leftSample = softClip(sampleValue * outputGain);
            rightSample = leftSample;
        }
        else
        {
            auto sample = engine.process();
            float primary = sample.primary;
            float secondary = sample.secondary;
            if (!(primary > -kSampleGuardLimit && primary < kSampleGuardLimit))
            {
                primary = 0.0f;
            }
            if (!(secondary > -kSampleGuardLimit && secondary < kSampleGuardLimit))
            {
                secondary = 0.0f;
            }
            if (kPreClipTanhDrive > 0.0f)
            {
                primary = std::tanh(primary * kPreClipTanhDrive);
                secondary = std::tanh(secondary * kPreClipTanhDrive);
            }
            leftSample = softClip(primary * outputGain * kGlobalPreGain);
            rightSample = softClip(secondary * outputGain * kGlobalPreGain);
        }
        uint16_t left = sampleToDac(leftSample);
        uint16_t right = sampleToDac(rightSample);
        audioBlock[i * 2] = left;
        audioBlock[i * 2 + 1] = right;
    }

    bool soundPlaying = engine.getIsPlaying();
    if (isHardware)
    {
        soundPlaying = true;
    }
    else if (isTest)
    {
        soundPlaying = engineGate;
    }
    gate.write(!soundPlaying);

    size_t bytesWritten = 0;
    if (!audioOut.write(audioBlock, sizeof(audioBlock), &bytesWritten))
    {
        ++underrunCount;
    }
    if (bytesWritten < sizeof(audioBlock))
    {
        ++underrunCount;
    }

    if (disyn::gStatusQueue != nullptr)
    {
        disyn::StatusMessage status{};
        status.underruns = underrunCount;
        status.audioOk = audioOk;
        xQueueOverwrite(disyn::gStatusQueue, &status);
    }
}

void Task(void *parameters)
{
    (void)parameters;
    Init();

    for (;;)
    {
        Tick();
    }
}

} // namespace disyn::dsp
