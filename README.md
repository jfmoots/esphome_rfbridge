# ESPHome RF Bridge

ESPHome RF Bridge is a reusable ESPHome external component for CC1101-based 433 MHz RF work.

The project is intended to act as a transport layer for Home Assistant integrations and future RF protocol decoders. The first target protocol is the Outprize RV roof vent fan remote.

## Current status

**v0.3.1 – RF Receive Pipeline**

Implemented:

- ESPHome external component loading from GitHub
- Native bit-banged SPI for CC1101
- CC1101 detection using PARTNUM/VERSION registers
- 433.92 MHz OOK async RX configuration
- GDO0 RF activity polling
- Packet candidate edge counting and duration logging
- Basic receive diagnostics

Not yet implemented:

- Protocol decoding
- Outprize packet reconstruction
- RF transmit
- Home Assistant service/actions

## Example YAML

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

## Expected v0.3.1 logs

On startup:

```text
Initializing CC1101 with native bit-banged SPI...
CC1101 reset complete
Detected CC1101 (PARTNUM=0x00 VERSION=0x14)
Configuring CC1101 for 433.92 MHz OOK async RX...
Entering CC1101 RX mode; listening for RF activity...
RX pipeline ready: polling GDO0 for async OOK edges
```

When RF activity is detected:

```text
RF packet candidate #1: edges=72 duration=25000 us rssi=-44 dBm
```
