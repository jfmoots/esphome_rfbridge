# Changelog

## v1.1.2 – Lambda TX Test Interface

Fixes first-transmit testing by avoiding the custom-action registration path that ESPHome was not recognizing in v1.1.1.

### Changed

- Updates firmware version to `1.1.2`.
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
