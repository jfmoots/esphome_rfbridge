# v1.1.3 – Configured Remote ID TX API

Fixes the lambda TX test interface by moving the Outprize remote ID into rfbridge configuration and adding a one-argument transmit helper.

Changes:
• Adds `remote_id:` to the `rfbridge:` configuration.
• Adds `send_outprize_low24(low24, repeats=3)` using the configured remote ID.
• Keeps the existing explicit `send_outprize_low24(remote_id, low24, repeats)` overload for advanced/manual tests.
• Updates TX test YAML to call `id(rf_bridge).send_outprize_low24(0x600140);`.
• Updates firmware version to 1.1.3.

# Changelog

## v1.1.3 – Lambda TX Test Interface

Fixes first-transmit testing by avoiding the custom-action registration path that ESPHome was not recognizing in v1.1.1.

### Changed

- Updates firmware version to `1.1.3`.
- Keeps the verified v1.0.0 Outprize receive decoder unchanged.
- Keeps the v1.1.x C++ transmit helpers unchanged.
- Changes transmit test YAML to call `id(rf_bridge).send_outprize_low24(...)` from ESPHome lambdas.
- Adds a Home Assistant API service example, `outprize_send_low24`, for arbitrary raw Low24 transmit testing.
- Keeps hard-coded template buttons only as smoke tests for known-good packets:
  - `0x600140` = 40% OUT
  - `0x600340` = 60% OUT
  - `0x600040` = fan off / awake idle
  - `0x600000` = power off

### Notes

The hard-coded buttons are not the final architecture. They are only for proving that the CC1101 transmit waveform can make the fan respond. The long-term interface is the raw Low24 API/service path, where the Home Assistant Outprize integration computes the desired packet and the RF bridge simply transmits it.
