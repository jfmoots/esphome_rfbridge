# Changelog

## v0.2.1 - RFBridge Pin Schema Sync

Fixes the ESPHome YAML validation schema for the native bit-banged SPI implementation introduced in v0.2.0.

Changes:
- Adds `sck_pin`, `mosi_pin`, and `miso_pin` to the RFBridge ESPHome config schema.
- Wires those pins into the RFBridge C++ component via `set_sck_pin`, `set_mosi_pin`, and `set_miso_pin`.
- Keeps `gdo0_pin` optional and `gdo2_pin` optional.
- Leaves the `spi:` block unused; RFBridge owns the CC1101 bit-banged SPI pins directly.

Expected YAML:

```yaml
rfbridge:
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
```

## v0.2.0 - CC1101 Bring-Up

Adds the first native CC1101 bring-up path for RF Bridge.

Changes:
- Adds CC1101 PARTNUM/VERSION register reads.
- Adds first-pass 433.92 MHz OOK async RX configuration.
- Uses a small GPIO bit-banged SPI layer to avoid the ESPHome/ESP-IDF SPI bus-lock crash seen in v0.1.x.
- Keeps RF transmit and packet decoding as placeholders for future milestones.
- Adds CC1101 bring-up documentation and example YAML.

