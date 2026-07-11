# Changelog

## v1.4.1

- Adds explicit logging of every complete-state API request and the resulting Low24 value.
- Normalizes requested speed to the nearest supported 10% step before encoding.
- Documents and warns that the vent field is a transient command nibble, not a persistent vent-state value.
- Warns when a powered-on complete-state request includes Vent Close (`0x04`) or Vent Stop (`0x0C`), because those actions can make the fan appear to power off.
- Keeps the verified power behavior: `powered: false` sends dedicated Power Off (`0x600000`); `powered: true` manufactures the requested awake/full-state frame.
- Updates the matching test procedure to use `vent_command: 0` for fan-only tests and `vent_command: 8` when explicitly opening the vent.

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
