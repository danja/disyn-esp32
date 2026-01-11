# Implementation Plan

## Goals
- Build ESP32-based Eurorack module firmware with dual-core split: UI on one core, DSP on the other.
- Support the listed hardware IO (OLED, encoder, gate IO, CV/pot ADCs, DAC outputs).
- Implement DSP algorithms based on `reference/disyn-lv2`.
- Provide UI for selecting algorithm and editing parameters (Attack, Decay, Reverb Size, Reverb Level).

## Assumptions
- Use PlatformIO in VS Code as the build environment.
- UI/DSP split mirrors `/home/danny/github/dreamachine` architecture.
- Use Adafruit OLED library with SSH1106 support; verify exact controller/variant on hardware.
- Target 44.1kHz sample rate if performance and I/O allow; validate on device.

## Phase 0: Project Setup
1. Initialize PlatformIO project for ESP32 DevKit V1 (DOIT).
2. Add dependencies for:
   - I2C OLED (Adafruit_SH110X preferred; fall back to Adafruit_SSD1306 if required by hardware).
   - Encoder handling (or custom GPIO-based).
   - Audio/DSP utilities (if required by ported algorithms).
3. Create basic folder layout: `src/`, `include/`, `lib/` for reusable modules.
4. Add a compile-time option for sample rate (default 44.1kHz) to allow quick testing.

## Phase 1: Hardware Abstraction Layer (HAL)
1. GPIO mapping constants from requirements.
2. Drivers:
   - OLED display (init, clear, render text/menus).
   - Encoder (rotation + pushbutton with debouncing).
   - Gate In (digital input, debounce if needed).
   - Gate Out (digital output).
   - ADC inputs for CV0-2 and Pot0-2 (calibration/scaling).
   - DAC outputs (write audio samples to DAC1/DAC2).
3. Add basic diagnostics screen showing live input values.

## Phase 2: UI Core
1. Menu model for:
   - Algorithm
   - Attack
   - Decay
   - Reverb Size
   - Reverb Level
2. Encoder switch cycles selection; encoder rotation edits value.
3. Define parameter ranges and default values.
4. Render UI to OLED (simple list with highlight).

## Phase 3: DSP Core
1. Port/reference all DSP algorithms from `reference/disyn-lv2` where feasible.
2. Define DSP parameter interface matching UI parameters.
3. Implement audio processing loop on the DSP core:
   - Read CV/pots as modulation inputs.
   - Update parameters at control rate.
   - Output to DAC1/DAC2.
4. Validate real-time performance and buffer scheduling at 44.1kHz; lower to 32kHz or 22.05kHz if needed.

## Phase 4: Dual-Core Integration
1. Assign UI task to one core, DSP task to the other.
2. Set up thread-safe parameter sharing (lock-free queue or atomic struct).
3. Ensure timing separation (UI refresh vs DSP audio loop).

## Phase 5: Calibration & Tuning
1. ADC scaling for CV and pot ranges.
2. DAC output scaling for audio range.
3. Encoder sensitivity and UI refresh tuning.

## Phase 6: Testing & Bring-up
1. Hardware smoke test for each IO.
2. UI navigation test.
3. DSP output test with known input.
4. Sample-rate validation (CPU load, buffer underruns, DAC stability).
5. Full system soak test.

## Deliverables
- `src/main.cpp` with core task scheduling.
- `src/ui/` module (display, menu, encoder input).
- `src/dsp/` module (algorithms, processing loop).
- `src/hal/` module (pins, ADC/DAC, IO drivers).
- `docs/PLAN.md` (this plan).

## Open Questions
- Confirm which Adafruit SH1106/SH1107 library variant compiles cleanly on ESP32.
- Validate 44.1kHz sample rate vs. CPU budget and DAC throughput on the DevKit V1.
- Confirm algorithm list and any exclusions from `reference/disyn-lv2`.
