# Changelog

## v0.4.0 – CC1101 Register Diagnostics

Adds detailed CC1101 register diagnostics to support receive-mode bring-up.

Changes:
- Adds full CC1101 configuration register dumps at multiple startup stages.
- Adds expected-value comparisons for key registers used by the current 433.92 MHz OOK async RX profile.
- Adds status register logging including RSSI, MARCSTATE, PKTSTATUS, and RXBYTES.
- Keeps v0.3.x firmware metadata, CC1101 detection, and receive pipeline diagnostics.

Notes:
- This release does not require pressing any RF remote.
- The goal is to compare the native ESPHome CC1101 configuration against the previously working diagnostic firmware / RadioLib-derived setup.
