#pragma once

#ifndef DISYN_SAMPLE_RATE
#define DISYN_SAMPLE_RATE 44100
#endif

constexpr int kSampleRate = DISYN_SAMPLE_RATE;

constexpr float kParamModAmount = 0.5f;
constexpr float kPitchCvMix = 0.5f;           // was .7
constexpr float kPitchPotMix = 0.5f;          // was .3
constexpr float kReverbSizeCvAmount = 0.0f;   // was 0.2f
constexpr float kReverbSizePotAmount = 0.0f;  //
constexpr float kReverbLevelCvAmount = 0.0f;  //
constexpr float kReverbLevelPotAmount = 0.0f; //
constexpr float kMasterCvAmount = 0.0f;
constexpr float kMasterPotAmount = 0.0f;
constexpr float kSampleGuardLimit = 8.0f; // was 8
constexpr float kGlobalPreGain = 0.7f;    // was 0.7
constexpr float kPreClipTanhDrive = 0.0f;
