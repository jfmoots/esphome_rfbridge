# Changelog

## v1.5.6 — Addressed STX882 Routing Fix

- Fixes the v1.5.5 addressed complete-state API path used by `ha_outprize`.
- Routes addressed Outprize commands through the same proven canonical waveform manufacturer and STX882 backend as the working single-remote API.
- Uses the supplied 11-bit remote ID only when constructing the transmitted 35-bit frame.
- Adds explicit logs for codec, selected TX backend, remote ID, Low24, and full35 frame.
- Keeps Outprize discovery events and receive behavior unchanged.
- Keeps all v1.5.4 diagnostic and single-remote APIs compatible.
- No YAML changes from v1.5.5.
