# Disyn ESP32

## Overview

Disyn ESP32 will be a Eurorack module based around an ESP32 module. The software architecture will be based on the model of /home/danny/github/dreamachine with one processor responsible for the user interface and the other for the DSP. The DSP algorithms will be based on those of reference/disyn-lv2
Coding will be done via PlatformIO in VS Code.

## Hardware

* Controller : ESP32 DevKit V1 DOIT module
* Display : SSH1106 based 1.3" I2C OLED
* Rotary Controller with Pushbutton Switch
* Digital Input GateIn from buffer
* Analog Inputs CV0 CV1 CV2 from buffers
* Analog Inputs Pot0 Pot1 Pot2 from potentiometers 
* Digital Output GateOut to buffer
* Analog Outputs DAC1 DAC2 to buffers

## ESP32 Connections

GND      Ov IN also to Display etc.
5v       +5v OUT to LED positive
3v3      3.3v OUT to Display etc.

### GPIO

25 DAC1
26 DAC2

21 Display SDA
22 Display SCL

16 Encoder 1 SW
34 Encoder 1 CLK
35 Encoder 1 DT

36 ADC CV0
39 ADC CV1
32 ADC CV2

33 ADC Pot0
27 ADC Pot1
4 ADC Pot2

18 Gate In
19 Gate Out

12 (JTAG TDI)
13 (JTAG TCK)
14 (JTAG TMS)
15 (JTAG TDO)

## User Interface

Pressing the encoder switch wll cycle through :

* Algorithm 
* Attack 
* Decay
* Reverb Size
* Reverb Level

Rotating the encoder on any will change the value.

