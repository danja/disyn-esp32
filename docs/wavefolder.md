# Wavefolder Plan + Findings (ESP32)

Goal: Add a wavefolder stage immediately after the oscillator, controlled by
knob/CV 1 (pot0/cv0), while keeping knob/CV 3 (pot2/cv2) on pitch.

## Changes attempted
- Added `WavefolderModule` and inserted it after the oscillator in
  `DisynEngine::process()`.
- Routed wavefold amount from pot0/cv0 in `DspTask.cpp`.
- Moved wavefolder out of the engine into `DspTask.cpp` (post-engine fold).
- Tried minimal fold math (no `fmod`/`fabs`) to rule out heavy math.
- Tried static local wavefolder vs member storage.
- Tried removing the wavefold amount setter call.

## Results
All variants above crashed on hardware (blank screen / no boot), even when the
wavefolder was a no-op or used only in `DspTask.cpp`.

## Current state
All wavefolder changes reverted; repo restored to last known-good boot state.

## Next steps (small-step reintroduction)
We will reintroduce wavefolder in tiny steps, flashing between each step to
confirm stability. Stop at the first crash and log the last-good step.

Step A: Baseline (done)
- No wavefolder code referenced; confirm boot stability.

Step B: Add module file only
- Add `WavefolderModule.hpp` but do not include it anywhere.

Step C: Include-only in DSP task
- `#include "dsp/modules/WavefolderModule.hpp"` in `DspTask.cpp`, no usage.

Step D: Instantiate-only in DSP task
- Add a `static WavefolderModule` in `DspTask.cpp`, no usage.

Step E: No-op call in DSP task
- Call `wavefolder.process(sample.primary, 0.0f)` and ignore the result.

Step F: Apply no-op passthrough
- Replace `primary`/`secondary` with the wavefolder output using amount 0.0f.

Step G: Use amount from pot0/cv0
- Pipe `wavefoldAmount` to the wavefolder in `DspTask.cpp` (post-engine).

Step H: Move wavefolder earlier
- Move wavefolder into `DisynEngine` after oscillator (minimal math).

Step I: Enable real fold math
- Switch to full folding implementation.

Notes:
- Add a short `Serial.println("WF: step X")` when changing steps to make
  boot diagnostics easier.
