# Changelog

## v0.3.0 - RF Receive Pipeline

- Adds first RF receive pipeline for CC1101 async OOK mode.
- Adds clearer CC1101 startup logging.
- Keeps native bit-banged SPI implementation.
- Polls GDO0 for RF edge activity and assembles packet candidates.
- Adds RX diagnostics in `dump_config()`:
  - RX enabled
  - packets seen
  - edges seen
  - overruns
  - last packet duration / edge count / RSSI
- Leaves protocol decoding and RF transmit as future milestones.

## v0.2.2 - CC1101 Bring-Up Logging

- Added CC1101 PARTNUM/VERSION visibility.
- Confirmed CC1101 detection/configuration status in ESPHome logs.

## v0.2.1 - RFBridge Pin Schema Sync

- Added SCK, MOSI, and MISO pins to the ESPHome schema.

## v0.1.x

- Established stable ESPHome external component foundation.
- Removed RadioLib from the ESPHome component path.
