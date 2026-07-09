# v1.3.13 – Learned OEM Edge Compare Diagnostics

This release adds diagnostics to compare the learned OEM Outprize pulse train against the ESP transmitter's generated pulse train.

Changes:
- Keeps v1.3.12.1 transmit timing, frequency trim helpers, CC1101 TX profile, calibration, OOK polarity, and RX restore unchanged.
- Stores the raw edge timing array from the OEM capture that produced `OUTPRIZE_LEARNED`.
- Stores the learned frame's decode start/stop indexes.
- Adds `compare_last_outprize_learned()` for Home Assistant YAML buttons.
- Logs a side-by-side OEM-vs-ESP timing comparison with per-edge differences.
- Logs OEM timing around the detected frame start and the ESP transmitter's generated preamble/start timing.
- Leaves the receiver and transmitter behavior unchanged; this is a diagnostics-only release.

Goal:
Determine whether the ESP-generated envelope is actually matching the OEM remote envelope, rather than only matching the decoded 35-bit frame value.
