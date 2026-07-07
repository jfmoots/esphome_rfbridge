# Hardware

## Known-good development hardware

- ESP32 dev board
- CC1101 433 MHz module
- 433 MHz antenna
- PlatformIO diagnostic firmware during reverse engineering
- ESPHome external component planned for production bridge

## CC1101 notes

The Outprize remote uses 433.92 MHz OOK-style signaling.

Initial diagnostic firmware settings used during captures:

```text
Frequency: 433.92 MHz
OOK: enabled
Bit rate: 2.0 kbps
RX bandwidth: 58.0 kHz
GDO0: async data mode
```

Document the exact wiring used by your build here before release.
