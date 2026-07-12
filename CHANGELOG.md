# Changelog

## v1.5.8

- Added a new random boot/session ID on every ESP boot.
- Added a generic `on_bridge_status` trigger.
- Bridge status includes boot ID, firmware version, and the full radio/codec capability contract.
- Bridge status is emitted every 10 seconds, allowing Home Assistant integrations to detect disconnects and automatic recovery without protocol entities on the ESP.
- Preserved all v1.5.7 Outprize receive, event, addressed transmit, fan-off/awake, and STX882 behavior.
- Removed bundled developer firmware and obsolete reverse-engineering examples from the production release package.
- Replaced the entity-heavy example with a production bridge example containing only hardware, transport events, and API actions.
