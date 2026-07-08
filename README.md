# ESPHome RF Bridge

ESPHome external component for a CC1101-based RF bridge used in the MooterHome Outprize vent fan reverse-engineering project.

## v1.0.0 focus

This release promotes the Outprize receive decoder to the first verified baseline. The protocol fields have been checked against live remote captures, including speed table entries, direction, rain, vent commands, POWER, and FAN behavior. The normal receive path remains protocol-oriented and quiet: it decodes first, logs compact protocol events, and only runs the expensive raw analyzer when `diagnostic_logging: true`.

RVLock is intentionally out of scope for RF rolling-code decoding in this project; that control path will use a cannibalized physical remote/button-push approach instead.

## ESPHome example

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
  diagnostic_logging: false
```

## Normal logging

With diagnostics off, successful Outprize captures are reduced to a compact event:

```text
OUTPRIZE low24=0x600340 confidence=excellent score=1155 candidates=12 edges=72 rssi=-49 dBm
```

Unknown captures are ignored at debug level unless diagnostics are enabled.

## Verified Outprize fields

- Fixed packet family: `0x60xxxx`
- Direction modifier: `+0x20`
- Rain modifier: `+0x10`
- Vent close/open/stop nibble: `+0x04`, `+0x08`, `+0x0C`
- POWER OFF: `0x600000`
- POWER ON / wake / fan-off awake: `0x600040`
- FAN button toggles fan-only: when off it sends remembered speed state; when on it sends `0x600040`.
