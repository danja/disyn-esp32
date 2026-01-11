#pragma once

#ifndef DISYN_SAMPLE_RATE
#define DISYN_SAMPLE_RATE 44100
#endif

constexpr int kSampleRate = DISYN_SAMPLE_RATE;

constexpr float kParamModAmount = 0.5f;
constexpr float kPitchCvMix = 0.7f;
constexpr float kPitchPotMix = 0.3f;
constexpr float kReverbSizeCvAmount = 0.2f;
constexpr float kReverbSizePotAmount = 0.2f;
constexpr float kReverbLevelCvAmount = 0.2f;
constexpr float kReverbLevelPotAmount = 0.2f;
constexpr float kMasterCvAmount = 0.1f;
constexpr float kMasterPotAmount = 0.1f;
