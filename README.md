# ESPHome RF Bridge v1.4.1

v1.4.1 is a diagnostic and API-semantics correction on the v1.4 transport baseline.

The complete-state API now logs its exact inputs and generated Low24 value. The `vent_command` field is an action nibble:

- `0` = no vent action
- `4` = close vent
- `8` = open vent
- `12` = stop vent

For a fan speed/direction/rain test that should not move the vent, use `vent_command: 0`. The earlier test used `4`, which intentionally requested Vent Close and could make the result look like a power-off failure.

The component still creates no native fan, cover, or switch entities. A separate Home Assistant integration will own those entities.

Use `esphome/examples/rfbridge_outprize_bridge.yaml` as the matching YAML/API contract.
