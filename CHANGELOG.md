# Changelog

## v1.5.5 — Outprize Discovery Events & Addressed Transport

- Emits every complete, valid Outprize frame through a generic `on_outprize_frame` trigger.
- Frame events are no longer limited to the legacy YAML `remote_id` used by temporary diagnostics.
- Event payload includes remote ID, Low24, power, speed, direction, rain, vent command, and receive RSSI.
- Adds a stateless addressed complete-state transmit method for multi-remote Home Assistant integrations.
- Preserves the legacy single-remote state cache and diagnostic sensors for v1.4/v1.5 testing.
- Preserves the verified Outprize codec, CC1101 receive path, canonical waveform manufacturer, and STX882 transmit path.
- Ignores self-reception during the existing post-transmit suppression window.
- No native fan, cover, rain, or protocol entities are added to the ESP.
