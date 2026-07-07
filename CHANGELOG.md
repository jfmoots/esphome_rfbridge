# Changelog

## v0.3.3 - Receive Gap Tuning

- Raises receive packet thresholds to avoid short background-noise candidates.
- Adds stale partial discard handling so sparse RF noise cannot slowly accumulate into fake packets.
- Adds min/avg/max gap statistics for packet candidates.
- Adds discarded partial diagnostics to dump_config output.
- Keeps v0.3.x CC1101 bring-up, version metadata, and receive pipeline intact.

## v0.3.2 - Build Metadata Logging

- Adds explicit build metadata to RFBridge logs.
- Reports firmware version, build date/time, and git reference in startup logs.
- Reports firmware version, build date/time, and git reference in `dump_config()`.
- Adds very-verbose per-edge RF logging for receive bring-up.
- Keeps the v0.3.x receive pipeline unchanged.

## v0.3.1 - Version Logging

- Adds `version.h` with a single RFBridge firmware version constant.
- Logs the RFBridge version during setup.
- Reports the RFBridge firmware version in `dump_config()`.
- Keeps the v0.3.0 receive pipeline unchanged.

## v0.3.0 - RF Receive Pipeline

- Adds first RF receive pipeline for CC1101 async OOK mode.
- Adds clearer CC1101 startup logging.
- Keeps native bit-banged SPI implementation.
- Polls GDO0 for RF edge activity and assembles packet candidates.
- Adds RX diagnostics in `dump_config()`.
- Leaves protocol decoding and RF transmit as future milestones.

## v0.2.2 - CC1101 Bring-Up Logging

- Added CC1101 PARTNUM/VERSION visibility.
- Confirmed CC1101 detection/configuration status in ESPHome logs.

## v0.2.1 - RFBridge Pin Schema Sync

- Added SCK, MOSI, and MISO pins to the ESPHome schema.

## v0.1.x

- Established stable ESPHome external component foundation.
- Removed RadioLib from the ESPHome component path.
