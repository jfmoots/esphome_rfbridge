# Changelog

## v0.7.0 – Pulse Histogram Analyzer

Adds automatic pulse-width analysis for RSSI-gated RF captures.

Changes:
- Keeps RSSI-gated fixed-window capture and raw timing output from v0.6.0.
- Adds per-capture pulse-width histograms using 64 us buckets.
- Adds dominant pulse bucket reporting to make protocol timing easier to spot.
- Updates firmware version metadata to 0.7.0.

## v0.6.0 – Raw Pulse Recorder

Adds raw timing output for RSSI-gated RF captures.

Changes:
- Keeps RSSI-gated fixed-window capture from v0.5.0.
- Logs accepted capture edge timing deltas in microseconds.
- Prints raw timings in compact 16-value chunks for copy/paste analysis.
- Updates firmware version metadata to 0.6.0.

## v0.5.0 – RSSI-Gated Capture

Reworked RF receive capture to match the original working diagnostic sniffer strategy.
