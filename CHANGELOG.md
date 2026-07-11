# Changelog

## v1.3.27 – Accepted Waveform Template & State Manufacturer

- Adds an edge-for-edge dump of the known-good SRX882 recording.
- Finds the embedded 35-bit Outprize payload within the accepted burst.
- Determines bit order/inversion and measured pulse/short-gap/long-gap timing.
- Adds a persistent in-RAM Outprize waveform template derived from the accepted Power Off recording.
- Adds manufactured arbitrary Low24 replay through STX882 while preserving the accepted header, trailer, levels, and pulse widths.
- Adds full-state manufactured replay using the canonical Gray-coded speed, direction, rain, and vent encoding.
- Keeps the v1.3.26 smart burst recorder unchanged.

# esphome_rfbridge v1.3.25

## v1.3.25 – SRX882/STX882 Raw RF Recorder

- Adds an explicit RF recorder using the proven SRX882 raw capture and STX882 raw replay path.
- Adds `start_rf_recorder(duration_ms)`.
- Adds `replay_rf_recording(repeats)`.
- Adds `clear_rf_recording()`.
- Adds Home Assistant-visible recorder availability, state, and recording summary helpers.
- Recording is deliberately gated and cannot be silently overwritten by ambient RF or later remote use.
- Keeps the v1.3.24 diagnostic and legacy transmit paths available in code.
