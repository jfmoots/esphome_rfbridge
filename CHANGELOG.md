# v1.2.1 – Build Metadata Compile Fix

Fixes v1.2.0 compile failure caused by missing build metadata macros.

Changes:
• Updates firmware version to 1.2.1.
• Defines safe fallback build metadata values in version.h.
• Keeps OEM waveform replay behavior from v1.2.0 unchanged.

# Changelog

## v1.2.1 – OEM Waveform Replay Test

- Updates firmware version to 1.2.1.
- Keeps the verified Outprize v1.0.0 decoder and v1.1.x TX helpers.
- Adds `replay_last_capture(repeats)` for raw RF waveform replay testing.
- Copies the last RSSI-gated capture and retransmits its pulse train through the CC1101 async OOK TX path.
- Adds an example ESPHome button: `RF Replay Last Capture`.
- Intended workflow: press an OEM remote button, then press the replay button to test whether raw waveform replay can control the fan before continuing generated-packet TX work.
