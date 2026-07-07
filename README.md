# ESPHome RF Bridge

Current release: **v0.8.0 – Capture Fingerprint Analyzer**

This release adds normalized pulse analysis and capture fingerprints on top of the RSSI-gated raw pulse recorder. Repeated RF transmissions can now be compared by fingerprint before protocol-specific decoding is implemented.

# ESPHome RF Bridge

ESPHome RF Bridge is a protocol-agnostic CC1101 RF bridge component for ESPHome.

## Current release: v0.7.0

This release adds a pulse histogram analyzer on top of the RSSI-gated capture path. When RSSI crosses the configured threshold, the component captures a fixed 140 ms GDO0 window and logs the edge timing deltas in microseconds.

This is intended to produce copyable timing data for protocol reverse engineering before adding protocol-specific decoders.
