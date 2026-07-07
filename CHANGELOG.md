# Changelog

## v0.5.0 – RSSI-Gated Capture

- Replaces continuous GDO0 packet assembly with RSSI-gated fixed-window capture.
- Arms only when RSSI reaches -80 dBm, matching the proven diagnostic sniffer strategy.
- Captures GDO0 for a fixed 140 ms window after an RSSI trigger.
- Logs discarded captures and accepted capture statistics.
- Keeps the v0.4.2 known-good CC1101 register profile.
