#pragma once

#include <cstdint>

namespace disyn {

struct AlgorithmParamInfo {
    const char *label;
    float minValue;
    float maxValue;
    bool integer;
};

struct AlgorithmInfo {
    const char *name;
    AlgorithmParamInfo param1;
    AlgorithmParamInfo param2;
};

constexpr AlgorithmInfo kDefaultAlgorithmInfo{
    "ALG",
    {"P1", 0.0f, 1.0f, false},
    {"P2", 0.0f, 1.0f, false},
};

constexpr AlgorithmInfo kAlgorithmInfoList[] = {
    {"Dirichlet", {"Harm", 1.0f, 64.0f, true}, {"Tilt", -3.0f, 15.0f, false}},
    {"DSF Single", {"Dec", 0.0f, 0.98f, false}, {"Rat", 0.5f, 4.0f, false}},
    {"DSF Double", {"Dec", 0.0f, 0.96f, false}, {"Rat", 0.5f, 4.5f, false}},
    {"Tanh Square", {"Drv", 0.05f, 5.0f, false}, {"Trim", 0.2f, 1.2f, false}},
    {"Tanh Saw", {"Drv", 0.05f, 4.5f, false}, {"Blend", 0.0f, 1.0f, false}},
    {"PAF", {"Form", 0.5f, 6.0f, false}, {"BW", 50.0f, 3000.0f, false}},
    {"Mod FM", {"Idx", 0.01f, 8.0f, false}, {"Rat", 0.25f, 6.0f, false}},
    {"Formant", {"Idx", 0.01f, 3.0f, false}, {"Space", 0.0f, 1.0f, false}},
    {"Cascade", {"DSF Dec", 0.5f, 0.95f, false}, {"Asym", 0.5f, 2.0f, false}},
    {"Banks", {"Idx", 0.01f, 8.0f, false}, {"Mix", 0.0f, 1.0f, false}},
    {"Feedback", {"Idx", 0.01f, 8.0f, false}, {"Fb", 0.0f, 0.95f, false}},
    {"Morphing", {"Morph", 0.0f, 1.0f, false}, {"Char", 0.0f, 1.0f, false}},
    {"Inharmonic", {"DSF Dec", 0.5f, 0.9f, false}, {"PAF Sh", 5.0f, 50.0f, false}},
    {"AFilter", {"Cut", 0.0f, 1.0f, false}, {"Res", 0.0f, 1.0f, false}},
    {"Multi", {"Tanh", 0.1f, 10.0f, false}, {"Exp", 0.1f, 1.5f, false}},
    {"Asym", {"LowR", 0.5f, 1.0f, false}, {"HiR", 1.0f, 2.0f, false}},
    {"Cross", {"M1", 0.0f, 1.0f, false}, {"M2", 0.0f, 1.0f, false}},
    {"Taylor", {"T1", 1.0f, 10.0f, true}, {"T2", 1.0f, 10.0f, true}},
    {"Trajectory", {"Sides", 3.0f, 12.0f, true}, {"Ang", 0.0f, 360.0f, false}},
    {"Hardware", {"Unused", 0.0f, 1.0f, false}, {"Unused", 0.0f, 1.0f, false}},
    {"TEST", {"Freq", 50.0f, 2000.0f, false}, {"Level", 0.0f, 1.0f, false}},
};

constexpr size_t kAlgorithmCount = sizeof(kAlgorithmInfoList) / sizeof(kAlgorithmInfoList[0]);
constexpr uint8_t kHardwareAlgorithmIndex = static_cast<uint8_t>(kAlgorithmCount - 2);
constexpr uint8_t kTestAlgorithmIndex = static_cast<uint8_t>(kAlgorithmCount - 1);

inline const AlgorithmInfo &GetAlgorithmInfo(uint8_t algorithm)
{
    if (algorithm < kAlgorithmCount)
    {
        return kAlgorithmInfoList[algorithm];
    }
    return kDefaultAlgorithmInfo;
}

inline float MapNormalized(const AlgorithmParamInfo &info, float normalized)
{
    if (normalized < 0.0f)
    {
        normalized = 0.0f;
    }
    if (normalized > 1.0f)
    {
        normalized = 1.0f;
    }
    return info.minValue + (info.maxValue - info.minValue) * normalized;
}

} // namespace disyn
