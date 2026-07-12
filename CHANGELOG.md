# Changelog

## v1.6.0 — Platform Edition

RF Bridge is now documented and presented as a general RF middleware platform rather than a single-device project.

### Documentation and project identity

- Rewrote the README as a complete platform overview and quick-start guide.
- Documented the separation between radio backends, protocol codecs, bridge transport, and Home Assistant integrations.
- Added hardware, installation, API, codec-development, repository-layout, architecture, and reverse-engineering guides.
- Documented the production CC1101 RX, STX882 TX, and SRX882 diagnostic roles.
- Documented the current Outprize codec and its relationship with `ha_outprize`.
- Preserved the canonical Outprize protocol findings in a dedicated technical reference.
- Clarified that a production bridge intentionally exposes no protocol-specific Home Assistant entities.
- Updated the example configuration to represent the complete v1.6.0 production contract.

### Firmware

- Updated firmware metadata to v1.6.0.
- Preserved the proven v1.5.9 runtime behavior and clean-build compatibility.
- No breaking changes to the Outprize event or API-action transport.

## Platform milestones

### v1.5.8–v1.5.9 — Production bridge lifecycle

- Added random boot/session ID, bridge heartbeat, firmware/capability status, and reconnect support.
- Removed entity-heavy production examples and protocol diagnostic polling.
- Made codec packaging safe for clean and incremental ESPHome builds.

### v1.5.5–v1.5.7 — Integration transport

- Added decoded frame events for unknown and configured remote IDs.
- Added addressed full-state transmission.
- Routed Outprize transmit through the proven STX882 backend.
- Added addressed fan-off/awake behavior for independent fan and vent control.

### v1.5.0–v1.5.4 — Codec architecture

- Established the generic codec interface.
- Compartmentalized Outprize protocol knowledge.
- Removed circular bridge/codec ownership.
- Added radio and codec capability advertisement.

### v1.4.x — State and command API

- Added complete-state and field-level transport testing.
- Proved OEM remote state synchronization and Home Assistant command transmission.
- Established full-state cache and command-source diagnostics for development.

### v1.3.x — Protocol and waveform discovery

- Established reliable CC1101 reception.
- Added SRX882 raw capture and STX882 transmission.
- Proved manufactured full-state commands.
- Replaced learned-capture dependency with a deterministic canonical waveform.
- Completed the Outprize protocol mapping used by the production codec.
