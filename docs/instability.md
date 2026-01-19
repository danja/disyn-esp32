# Algorithm Instability Issue

## Symptom
Some algorithms produce what sounds like a loud square wave instead of their intended output. Tanh Sq works fine, but Tanh Saw and potentially other algorithms exhibit this behavior.

## Root Cause
The algorithms output signals with varying amplitude ranges. Some (like TanhSaw) can output values up to ±2 or higher, while others (like TanhSquare) apply internal limiting.

When hot signals hit the `softClip()` function in DspTask.cpp, `tanh()` saturates them to near ±1, which sounds like a square wave when the saturation is constant.

## Changes Made

### DspTask.cpp
- Removed hard clipping before `tanh()` in `softClip()` - was defeating soft clipping purpose
- Original code clamped to ±1 then applied `tanh(value * 1.2)`, now just uses `tanh(value)`
- Added a range guard before `softClip()` to zero out NaN/Inf or absurdly large
  samples using simple comparisons only (ESP32-safe).
- The guard limit is configurable via `kSampleGuardLimit` in `include/Config.h`.

### TanhSawAlgorithm.hpp
- Added `tanh()` to final output to match TanhSquare's behavior
- Was: `output = square * (1-blend) + saw * blend`
- Now: `output = tanh(square * (1-blend) + saw * blend)`

## Still To Do
- Review other algorithms for similar hot output issues
- Algorithms to check: DSF variants, Combination algorithms, Novel algorithms
- Consider adding output limiting to OscillatorModule::process() as a safety net
- Note: `std::isfinite()` caused system crashes on ESP32 - avoid using it
- Additional note: attempts to add NaN/Inf guards and output sanitizers in
  `OscillatorModule::process()` have caused blank screen/system crashes on
  ESP32. See `docs/danger-zones.md` before reattempting.

## Testing
Select "Hardware" algorithm for a known-good 440Hz square wave reference. Use "TEST" algorithm with adjustable frequency/level for debugging.
