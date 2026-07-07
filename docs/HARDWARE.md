# Hardware

## Known-good development hardware

- ESP32 dev board
- CC1101 433 MHz module
- 433 MHz antenna
- PlatformIO diagnostic firmware during reverse engineering
- ESPHome external component for the production bridge

## Wiring used during development

| CC1101 | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CSN / CS | GPIO5 |
| GDO0 | GPIO4 |
| SCK | GPIO18 |
| MOSI / SI | GPIO23 |
| MISO / SO | GPIO19 |

GPIO5 is an ESP32 strapping pin. It worked in the prototype, but future hardware builds should consider moving CS to a non-strapping GPIO.

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

The ESPHome component is intentionally moving away from RadioLib and toward a small native CC1101 driver based on ESPHome's SPI component.
