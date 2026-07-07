# ESPHome RF Bridge

`esphome_rfbridge` is a reusable ESPHome RF transport layer for CC1101-based 433 MHz devices.

The project is intended to act as a hardware appliance for Home Assistant integrations. It should own the RF hardware, while Home Assistant integrations own device-specific user models such as fans, lights, locks, and switches.

## Current status

v0.2.2 is a CC1101 bring-up schema fix release. It should boot in ESPHome, validate the bit-banged SPI pin YAML, initialize the CC1101, read `PARTNUM` and `VERSION`, and apply the first Outprize-compatible 433.92 MHz OOK async RX register configuration.

Transmit, packet capture, and protocol decoding are not implemented yet.

## Example

```yaml
external_components:
  - source: github://jfmoots/esphome_rfbridge
    components: [rfbridge]

rfbridge:
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
```

GPIO5 is a strapping pin on ESP32. It worked during development, but future hardware builds should consider moving CS to a non-strapping GPIO.


## v0.2.2 – CC1101 Bring-Up Logging

This release adds visible CC1101 bring-up status in the ESPHome logs and keeps the bridge online if the CC1101 is not detected.
