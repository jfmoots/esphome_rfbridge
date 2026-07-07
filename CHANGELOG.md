# Changelog

## v0.10.0 – Outprize Candidate Decoder

Adds first protocol-specific decoder output for Outprize RF captures.

Changes:
- Keeps the RSSI-gated fixed-window capture pipeline.
- Keeps raw pulse, histogram, fingerprint, normalized stream, and symbol analyzer output.
- Adds Outprize PWM gap candidate detection.
- Decodes short-gap/long-gap pulse pairs into bits.
- Logs binary, grouped hex, and Low24 values for comparison with previous captures.
- Stores edge levels during capture for future signed raw-output work.

