# v1.3.11 – OEM Capture Learn & Replay

This release adds a capture-to-bits learning path for Outprize protocol reconstruction.

## Changes
- Keeps the v1.3.8/v1.3.10 transmit timing, CC1101 TX profile, calibration, OOK polarity, and RX restore path unchanged.
- Expands the Outprize receive decoder so full RSSI-window captures containing repeated frames can still be scanned for a valid 35-bit candidate.
- Stores the best decoded 35-bit OEM frame as the current learned Outprize frame.
- Logs `OUTPRIZE_LEARNED` with the recovered full35 value, remote prefix, low24, bit count, score, and binary stream.
- Adds `replay_last_outprize_learned()` for Home Assistant YAML buttons.
- Adds `clear_last_outprize_learned()` for clearing the learned frame during testing.

## Goal
Stop guessing the frame from `prefix + low24` and allow direct replay of the most recently decoded OEM Outprize frame.
