# ESPHome RF Bridge

ESPHome RF Bridge is a protocol-agnostic CC1101 RF bridge component for ESPHome.

## Current release: v0.6.0

This release adds a raw pulse recorder on top of the RSSI-gated capture path. When RSSI crosses the configured threshold, the component captures a fixed 140 ms GDO0 window and logs the edge timing deltas in microseconds.

This is intended to produce copyable timing data for protocol reverse engineering before adding protocol-specific decoders.
