# Hardware Guide

## Reference build

The proven bridge contains:

- ESP32 DevKit
- CC1101 433 MHz module with antenna
- STX882 ASK/OOK transmitter
- SRX882 ASK/OOK receiver
- 3.3 V power distribution and decoupling
- Separate receive/transmit antennas
- Enclosure with the RF elements kept clear of large metal surfaces

## Reference pin map

| Module signal | ESP32 pin | Purpose |
|---|---:|---|
| CC1101 CS | GPIO5 | SPI chip select |
| CC1101 SCK | GPIO18 | SPI clock |
| CC1101 MOSI | GPIO23 | SPI data to CC1101 |
| CC1101 MISO | GPIO19 | SPI data from CC1101 |
| CC1101 GDO0 | GPIO4 | Primary data/interrupt line |
| CC1101 GDO2 | GPIO27 | Auxiliary data line |
| STX882 DATA | GPIO26 | Direct ASK/OOK transmit waveform |
| SRX882 DATA | GPIO25 | Diagnostic raw receive waveform |
| SRX882 CS/EN | GPIO33 | Receiver enable/control |

GPIO5 is an ESP32 strapping pin. The reference build works with it as CC1101 CS, but avoid external pull resistors or circuitry that forces an incorrect level during boot.

## Power

- Use 3.3 V for CC1101, STX882, and SRX882 unless the exact module documentation explicitly states otherwise.
- Share ground among all modules and the ESP32.
- Place local decoupling near RF modules. A practical starting point is 100 nF ceramic in parallel with 10 µF bulk capacitance at the module supply rails.
- Avoid powering RF modules from noisy or poorly regulated rails.

## Module mounting

Socketed or connectorized modules make replacement and experimentation easier:

- Solder female headers to the perfboard or carrier board.
- Solder male pins to the RF modules.
- Keep SPI and data leads short.
- Provide strain relief for antennas and enclosure feed-throughs.

## Antennas

At 433.92 MHz, the free-space quarter-wave length is approximately:

```text
17.3 cm / 6.8 in
```

Measure from the electrical antenna feed point. Bends, knots, nearby ground, enclosure material, and module layout change the effective tuning.

Practical guidance:

- Keep wire antennas as straight as possible.
- Vertical orientation usually matches handheld remote polarization well.
- Separate the STX882 antenna from the CC1101 receive antenna by several inches when possible.
- Do not bundle antenna wire with power or signal wiring.
- Keep antennas away from large metal objects and foil-backed insulation.

## Placement

A central, hidden location is ideal for an RV installation:

- Reduces maximum distance to remotes and target devices
- Avoids long coax runs
- Keeps the bridge protected and visually unobtrusive
- Allows one bridge to serve multiple protocols throughout the vehicle

## Current radio roles

### CC1101

Used as the normal Outprize receiver because it provides configurable frequency, bandwidth, RSSI, and interrupt-driven capture.

### STX882

Used as the normal Outprize transmitter. Its simple direct ASK/OOK data path reliably reproduces the accepted envelope for the fan receiver.

### SRX882

Used for raw capture and diagnostic comparison. It was instrumental in establishing the canonical waveform but is not required for normal Outprize operation once the codec is known.

## Safety and regulatory notes

- Use legal frequencies, duty cycles, and transmit power for the installation region.
- Do not transmit commands to safety-critical equipment without suitable safeguards.
- Maintain electrical isolation and proper fusing when powering the bridge from vehicle systems.
