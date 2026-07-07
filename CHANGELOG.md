# Changelog

## v0.8.0 – Capture Fingerprint Analyzer

- Keeps RSSI-gated fixed-window capture and raw pulse recorder.
- Adds filtered protocol-analysis pass that ignores pulses below 50 µs and above 2000 µs.
- Adds normalized pulse stream output using 64 µs buckets.
- Adds FNV-1a capture fingerprint so repeated transmissions can be compared quickly.
- Adds filtered symbol count and unique bucket count to diagnostics.
