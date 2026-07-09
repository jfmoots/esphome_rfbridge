# v1.3.10 – Implemented Power Off Frame Test Helpers

This release fixes the v1.3.9 test-helper mismatch.

Changes:
- Adds the public C++ helper methods used by the YAML protocol-test buttons.
- Adds Power Off LSB, inverted MSB, inverted LSB, and raw OEM Power Off helpers.
- Keeps the v1.3.8/v1.3.9 RF timing, CC1101 TX profile, calibration, OOK polarity, and RX restore path unchanged.
- Leaves the receiver and decoder path unchanged.

Goal:
Allow protocol-content testing from Home Assistant without asking users to call functions that are not present in the component.
