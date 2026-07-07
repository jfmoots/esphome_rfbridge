# Changelog

## v0.2.0 - CC1101 Bring-Up

- Adds first native CC1101 bring-up path.
- Reads and logs CC1101 `PARTNUM` and `VERSION` registers.
- Configures known-good 433.92 MHz OOK async RX baseline values from the diagnostic firmware.
- Avoids ESPHome SPI bus-lock startup crashes by using a small GPIO bit-banged SPI layer for initial bring-up.
- Keeps TX and packet decode as placeholders for future milestones.

## v0.1.3 - Stable ESPHome Foundation

- Stable ESPHome baseline with API, OTA, web server, and logging.
- RF functionality intentionally disabled pending CC1101 bring-up.

## v0.1.1 - Native CC1101 Foundation

- Removed RadioLib dependency from the ESPHome component path.
- Started native CC1101 driver foundation.
