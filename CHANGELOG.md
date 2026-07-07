# Changelog

## v0.2.2 – CC1101 Bring-Up Logging

- Adds explicit INFO-level CC1101 initialization logs.
- Stores CC1101 PARTNUM/VERSION so they appear in `dump_config()`.
- Keeps ESPHome online if CC1101 bring-up fails instead of marking the component failed.
- Reports whether the CC1101 was detected and configured.

## v0.2.1 – RFBridge Pin Schema Sync

- Adds `sck_pin`, `mosi_pin`, and `miso_pin` to the ESPHome schema.
