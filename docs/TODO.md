# Unstable algorithms

The following suffer from instability, sound very bad, I believe caused by NaN values or similar. Guards should be put in place as necessary (ESP32 doesn't support isInfinity - check) :

- DSF Single
- DSF Double
- Tanh Saw
- Mod FM
- Formant
- Cascade
- Banks
- Feedback
- Morphing
- AFilter
- ASym
- Cross
- Taylor

Trajectory algorithm doesn't seem to follow its specification.

The Hardware 'algorithm' screen can be removed. Leave TEST in place.

# Rotary Debounce

Currently the rotary controller seems unreliable, debouncing needs improving.

# Missing Controls

Pot 1, CV1 (param 1) and CV0 (wavefolding) currently don't appear to do anything

# Pitch

Scaling/offset for pitch is needed. We need a calibration menu screen. Currently the value is only changing with the CV with value around 10-20%, the knob with values around 0-40%

The knob decreases pitch with increasing value. This is ok for the CV, that has an inverter in hardware, but the knob doesn't.

# Oscilloscope

A screen should be added to show an oscilloscope display of the value going into CV1. The knobs should control amplitude scale, offest and timebase frequency.
