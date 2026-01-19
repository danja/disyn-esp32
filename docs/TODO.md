 Please build/flash and test those unstable algorithms again. If they’re still too hot, increase
  kAlgorithmLimitDrive (e.g. 2.0–3.0). If they sound too damped, lower it (e.g. 1.2).

2026-01-19

# Unstable algorithms

See docs/instability.md

The following suffer from intermittentent instability, I believe caused by NaN values or similar. Guards should be put in place as necessary (ESP32 doesn't support isInfinity - check) :

- Dir Pulse
- Tanh Saw
- PAF
- Mod FM
- C1 Hyb
- C2 Cas
- C3 Par
- C4 Fdb
- C5 Mor
- C7 Flt
- N2 Asy

# Naming on display

The "Alg" characters should be removed so the title line just shows the name of the algorithm. These should be named as follows (best effort to fit on line, crop if necessary):

- Dir Pulse: Dirichlet
- DSF S: DSF Single
- DSF D: DSF Double
- Tanh Sq: Tanh Square
- Tanh Saw: Tanh Saw
- PAF: PAF
- Mod FM: Mod FM
- C1 Hyb: Formant
- C2 Cas: Cascade
- C3 Par: Banks
- C4 Fdb: Feedback
- C5 Mor: Morphing
- C6 Inh: Inharmonic
- C7 Flt: AFilter
- N1 Mul: Multi
- N2 Asy: Asym
- N3 XMod: Cross
- N4 Tay: Taylor
- Traj: Trajectory

# More lines on display

It looks like there are 3 or 4 lines still available on the display (check the library). These should be put to use showing the parameter names and values

# Use of Both Channels

Where the algorithm includes a secondary value that may be easily extracted, this shoud be sent to DAC 2

# Maximal use of CV and Knobs

All three knobs and all three CV inputs should have some effect on every algorithm. If any are unused, check the parameters currently assigned to the rotary encoder, shift across as necessary.

# Rotary Debounce

Currently the rotary controller seems unreliable when setting variable values (seems ok for algorithm selection). Check if extra debouncing is needed.

# Gate Out

gate out should follow the inverted value of if there is a sound currently playing

# A screen should be added to show an oscilloscope display of the value going into CV1. The knobs should control amplitude scale, offest and timebase frequency.
