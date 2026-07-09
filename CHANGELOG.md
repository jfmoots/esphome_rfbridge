# v1.3.14 – Version Macro Compile Fix & Compare Diagnostics

This release fixes the compile failure in v1.3.13 caused by inconsistent RFBridge version macro naming.

Changes:
• Defines RFBRIDGE_VERSION, RFBRIDGE_FIRMWARE_VERSION, RFBRIDGE_BUILD_DATE, RFBRIDGE_BUILD_TIME, and RFBRIDGE_GIT_REF in a static version header.
• Keeps the v1.3.13 learned OEM edge-compare diagnostics unchanged.
• Keeps v1.3.12.1 transmit timing, frequency trim helpers, CC1101 TX profile, calibration, OOK polarity, and RX restore unchanged.
• Leaves the receiver and transmitter behavior unchanged.

Goal:
Make the learned-vs-transmitted waveform comparison build compile cleanly so testing can continue without changing the RF experiment.

# v1.3.14 – Learned OEM Edge Compare Diagnostics

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
