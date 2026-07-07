# ESPHome RF Bridge

Current release: **v0.9.0 – Symbol Stream Analyzer**

ESPHome external component for a CC1101-based RF bridge.

## v0.9.0 highlights

- Keeps RSSI-gated fixed-window capture.
- Keeps raw pulse timing, histograms, normalized pulses, and fingerprints.
- Adds compact symbol alphabet output for each capture.
- Adds symbol stream output to make repeated patterns easier to see.
- Adds run-length compressed symbol output.
- Adds simple repeated motif detection.

## Example ESPHome YAML

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
