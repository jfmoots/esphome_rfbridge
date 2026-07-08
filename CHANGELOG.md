# Changelog

## v1.3.1 - TX Strength + State Logging

- Keeps the verified Outprize decoder intact.
- Keeps the experimental OOK TX path from v1.3.0.
- Uses CC1101 PA table `0xC0` for maximum test output.
- Increases example TX repeats to 8 for stronger SDR visibility.
- Logs estimated TX duration for Outprize and OOK test bursts.
- Logs CC1101 `MARCSTATE` around TX transitions and `STX` status.
- Restores RX mode after transmit tests.

# Changelog

## v1.3.0 - TX Hardware Verification

- Adds `send_ook_test_burst(pulse_us, pulse_count, repeats)` for protocol-neutral CC1101 TX bring-up.
- Adds an example `RF TX Hardware Test Burst` ESPHome template button.
- Keeps the verified Outprize decoder and existing experimental Outprize TX helpers intact.
- Leaves packet synthesis untouched; this release is intended to verify that the RF hardware emits a 433.92 MHz OOK burst that can be seen with an SDR/sniffer.
- Restores RX mode after every test burst.

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
