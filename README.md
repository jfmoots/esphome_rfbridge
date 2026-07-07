# ESPHome RF Bridge

Reusable ESPHome RF bridge framework for CC1101-based 433 MHz devices.

This project is intentionally transport-focused. It owns the radio hardware and provides packet send/receive plumbing. Device-specific Home Assistant integrations, such as `ha_outprize`, should own user-facing entities and business logic.

## Goals

- CC1101 RF transmit/receive bridge for ESPHome
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

## Status

Early development. The Outprize protocol has been reverse engineered and is being used as the first real protocol implementation.
