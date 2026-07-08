# Changelog

## v1.1.1 – ESPHome TX Actions

Fixes the YAML automation interface for the first Outprize transmitter.

### Added
- Registers `rfbridge.send_outprize_low24` as an ESPHome automation action.
- Registers `rfbridge.send_outprize_fan_off` and `rfbridge.send_outprize_power_off` convenience actions.
- Adds action examples using `id: rf_bridge` so template buttons can trigger TX without C++ lambdas.

### Notes
- The verified Outprize protocol model is unchanged.
- The v1.1.0 C++ transmit helpers remain available for lambda-based testing.
- This release exists because v1.1.0 added TX helpers but did not expose the custom YAML action used by the test buttons.

## v1.1.0 – First Outprize Transmitter

Adds the first experimental Outprize RF transmit path.

### Added
- Experimental CC1101 async OOK transmit support.
- `send_outprize(...)` helper for speed, direction, rain, and vent commands.
- `send_outprize_low24(...)` helper for verified raw Low24 packets.
- Convenience helpers for POWER OFF (`0x600000`) and FAN OFF / awake idle (`0x600040`).
- Verified Outprize speed table is now used by the encoder.
- Example YAML template buttons for initial transmit testing.

### Notes
- Receive decoder behavior from v1.0.0 is unchanged.
- Transmit uses the verified 35-bit Outprize frame shape: fixed prefix + Low24 payload.
- This is a first RF transmit build intended for careful bench/coach testing.
