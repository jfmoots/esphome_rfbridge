# Changelog

## v1.3.29 — Native Outprize entities and OEM remote synchronization

- Adds a native 10-speed ESPHome fan entity with direction control.
- Adds a native vent cover entity with open, close, and stop commands.
- Adds a native rain-sensor switch.
- Maintains one shared full-state Outprize cache for all entities.
- Decodes valid 0x6CF OEM remote frames and atomically updates Home Assistant.
- Treats the OEM remote as authoritative when a valid full-state frame is received.
- Publishes the last command source: Home Assistant, OEM Remote, or Startup.
- Uses the built-in canonical STX882 waveform manufacturer from v1.3.28.
- Temporarily suppresses self-reception after local STX882 transmissions.
- Retains the SRX882 recorder and CC1101 diagnostics for future RF work.


## v1.3.28 — Built-in Outprize Waveform Manufacturer

- Packages the accepted Outprize waveform structure directly in firmware.
- Removes the requirement to record or analyze an OEM command before normal transmission.
- Generates the full 35-bit frame MSB-first using remote prefix `0x6CF`.
- Uses the learned canonical envelope: 8 fixed header edges, 500 µs data pulses, 500 µs zero gaps, and 1500 µs one gaps.
- Keeps the SRX882 recorder and waveform analyzer as diagnostic tools only.
- Existing manufactured-command APIs now use the built-in generator automatically.
- Commands work immediately after boot or firmware update without any captured template in RAM or flash.
