# ESPHome RF Bridge

ESPHome external component for a CC1101-based RF bridge used in the MooterHome Outprize vent fan reverse-engineering project.

## v0.10.1 focus

This release is a cleanup and robustness pass for the verified Outprize decoder path. The normal log path now prioritizes decoded `Low24` packet output. Full raw edge timings, histograms, symbol streams, and protocol-analyzer output are still available by setting `diagnostic_logging: true` in the `rfbridge:` block.

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

## Verified Outprize fields

- Fixed packet family: `0x60xxxx`
- Direction modifier: `+0x20`
- Rain modifier: `+0x10`
- Vent close/open/stop nibble: `+0x04`, `+0x08`, `+0x0C`
- POWER OFF: `0x600000`
- POWER ON / wake / fan-off awake: `0x600040`
- FAN button toggles fan-only: when off it sends remembered speed state; when on it sends `0x600040`.
