# esphome_rfbridge v1.3.25

## v1.3.25 – SRX882/STX882 Raw RF Recorder

- Adds an explicit RF recorder using the proven SRX882 raw capture and STX882 raw replay path.
- Adds `start_rf_recorder(duration_ms)`.
- Adds `replay_rf_recording(repeats)`.
- Adds `clear_rf_recording()`.
- Adds Home Assistant-visible recorder availability, state, and recording summary helpers.
- Recording is deliberately gated and cannot be silently overwritten by ambient RF or later remote use.
- Keeps the v1.3.24 diagnostic and legacy transmit paths available in code.
