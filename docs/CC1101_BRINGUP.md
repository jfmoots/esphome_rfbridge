# CC1101 Bring-Up

v0.2.0 validates basic communication with the CC1101 without RadioLib.

Expected useful log lines:

```text
[rfbridge] Setting up RF Bridge v0.2.0...
[rfbridge] Initializing CC1101 with native bit-banged SPI...
[rfbridge] CC1101 PARTNUM=0x00 VERSION=0x14
[rfbridge] Configuring CC1101 for 433.92 MHz OOK async RX...
[rfbridge]   IOCFG0   = 0x0D
[rfbridge]   IOCFG2   = 0x2E
[rfbridge]   PKTCTRL0 = 0x32
[rfbridge] RF Bridge setup complete
```

Common CC1101 modules often report `VERSION=0x14` or `VERSION=0x04`. A value of `0x00` or `0xFF` usually means the CC1101 did not respond correctly.

## Why bit-banged SPI first?

The initial ESPHome-native SPI attempt exposed an ESP-IDF bus-lock assertion during startup. v0.2.0 uses a minimal GPIO bit-banged SPI layer to validate the radio hardware and register configuration without triggering ESPHome SPI transaction handling. The bridge can revisit native SPI after the low-level CC1101 behavior is proven.
