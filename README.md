# ESPHome RF Bridge

Reusable ESPHome RF bridge framework for CC1101-based 433 MHz devices.

This project is intentionally transport-focused. It owns the radio hardware and provides packet send/receive plumbing. Device-specific Home Assistant integrations, such as `ha_outprize`, should own user-facing entities and business logic.

## Goals

- CC1101 RF transmit/receive bridge for ESPHome
- Native ESPHome SPI integration
- No RadioLib runtime dependency
- Protocol abstraction layer
- Outprize RV vent fan protocol as the first supported protocol
- Remote learning support
- Diagnostic firmware retained separately for packet discovery

## Repository layout

```text
esphome_rfbridge/
├── docs/
│   ├── ARCHITECTURE.md
│   ├── HARDWARE.md
│   └── OUTPRIZE_PROTOCOL.md
├── esphome/
│   ├── components/
│   │   └── rfbridge/
│   └── examples/
├── tools/
└── diagnostic_firmware/
```

## ESPHome usage

```yaml
external_components:
  - source: github://jfmoots/esphome_rfbridge
    components: [rfbridge]

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

rfbridge:
  id: rf_bridge
  cs_pin: GPIO5
  gdo0_pin: GPIO4
```

Do not add RadioLib to `platformio_options`. The bridge is being implemented with ESPHome's native SPI support.

## Status

Early development. The Outprize protocol has been reverse engineered. The current component initializes the CC1101 directly over ESPHome SPI and logs basic radio identity/configuration. RF transmit and receive are next.

## Release Notes

### v0.1.2 - SPI Bus Lock Crash Fix

Fixes an ESPHome boot-loop caused by an unpaired SPI `disable()` call during the CC1101 reset sequence. The RF Bridge now uses paired ESPHome SPI transactions during startup, avoiding the ESP-IDF `spi_device_release_bus` assertion.

### v0.1.1 - Native CC1101 Foundation

Removed RadioLib from the ESPHome component path and began the native CC1101 driver foundation.
