# Changelog

## v1.3.9 – Frame Content Diagnostics

This release keeps the v1.3.8 waveform timing unchanged and adds packet-content diagnostics/test modes.

Changes:
- Logs the exact 35-bit transmit stream before each Outprize transmission.
- Adds alternate Power Off test-call helpers for LSB order, inverted bits, and LSB inverted bits.
- Adds raw full35 transmit helpers for direct frame replay experiments.
- Keeps CC1101 TX profile, PATABLE, SCAL calibration, OOK polarity, and RX restore unchanged.
- Leaves the receiver and decoder path unchanged.

Goal:
Determine whether the remaining fan-control failure is caused by bit order, inversion, or full-frame construction rather than RF timing.
