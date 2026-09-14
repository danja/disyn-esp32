#pragma once

constexpr int kPinDac1 = 25;
constexpr int kPinDac2 = 26;

constexpr int kPinI2cSda = 21;
constexpr int kPinI2cScl = 22;

// Encoder moved off 34/35 so those ADC1 pins are free for pots.
// 13/14 support internal pull-ups; 12 and 15 are strapping pins, avoid them.
constexpr int kPinEncSw = 16;
constexpr int kPinEncClk = 14;
constexpr int kPinEncDt = 13;

// All six analog inputs are on ADC1. ADC2 is unusable here because the
// I2S built-in DAC drives the same RTC/SAR block on GPIO25/26.
constexpr int kPinCv0 = 36; // ADC1_CHANNEL_0
constexpr int kPinCv1 = 39; // ADC1_CHANNEL_3
constexpr int kPinCv2 = 32; // ADC1_CHANNEL_4

constexpr int kPinPot0 = 33; // ADC1_CHANNEL_5
constexpr int kPinPot1 = 34; // ADC1_CHANNEL_6
constexpr int kPinPot2 = 35; // ADC1_CHANNEL_7

constexpr int kPinGateIn = 18;
constexpr int kPinGateOut = 19;
