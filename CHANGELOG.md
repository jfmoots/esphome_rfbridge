# v1.3.12 – TX Frequency Trim Tests

This release adds transmit frequency trim experiments for Outprize fan control.

v1.3.11 confirmed that the learned OEM Power Off frame matches the previously reconstructed frame:
`full35=0x6CF600000`, `remote=0x6CF`, `low24=0x600000`.

Since frame contents and waveform timing now appear close, the next hypothesis is RF frequency alignment. SDR captures showed the OEM remote and ESP transmitter centered differently, potentially by several tens of kHz.

Changes:
- Keeps the existing Outprize frame content unchanged.
- Keeps the v1.3.8+ waveform timing unchanged.
- Keeps the CC1101 TX profile, calibration, OOK polarity, and RX restore path unchanged.
- Adds Power Off transmit helpers at multiple frequency trims around 433.92 MHz.
- Adds logging showing the selected TX frequency profile before transmission.
- Leaves the receiver and decoder path unchanged.

Goal:
Determine whether the fan receiver responds when the ESP transmission is shifted closer to the OEM remote’s apparent RF center frequency.
