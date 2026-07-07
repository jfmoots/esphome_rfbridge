# ESPHome RF Bridge

Reusable ESPHome RF bridge foundation for CC1101-based 433 MHz devices.

## v0.4.0 Focus

This release adds CC1101 register diagnostics. It is intended to verify that the ESPHome-native driver is programming the radio into the same effective mode as the earlier standalone diagnostic firmware.

Expected healthy markers:

```text
Firmware Version: 0.4.0
CC1101 Detected: YES
CC1101 Configured: YES
CC1101 PARTNUM: 0x00
CC1101 VERSION: 0x14
RX Enabled: YES
```

The log will also include register dumps for:

- post-reset / pre-config
- post-config / pre-RX
- post-RX

No remote button press is required for this release.

## ESPHome YAML

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/jfmoots/esphome_rfbridge
      ref: main
      path: esphome/components
    refresh: 0s
    components: [rfbridge]

rfbridge:
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
```
