# Changelog

## v1.4.0

- Discards the experimental v1.3.29 native ESPHome fan/cover entity approach.
- Establishes a stable RF transport and full-state API for a separate Home Assistant integration.
- Adds one shared Outprize requested-state cache.
- Adds atomic complete-state and field-level setters for power, speed, direction, rain, and vent command.
- Routes all Home Assistant-originated changes through the built-in canonical STX882 waveform manufacturer.
- Updates the same cache from valid CC1101-decoded OEM remote frames with the configured 11-bit prefix.
- Adds command-source, revision, validity, Low24, and state-summary getters.
- Adds short self-reception suppression following STX882 transmission.
- Includes matching ESPHome API actions and diagnostic state entities in the example YAML.
- Creates no fan, cover, or switch entities inside the ESPHome external component.
