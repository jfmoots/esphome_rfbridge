# ESPHome RF Bridge

ESPHome external component for an ESP32 + CC1101 RF bridge.

## v0.10.0

This release adds the first Outprize-specific decoder pass on top of the generic RF analyzer.

The receiver still uses the proven RSSI-gated capture strategy:

- arm when RSSI rises above -80 dBm
- capture GDO0 for 140 ms
- log raw timings, histograms, normalized symbols, fingerprints, and motifs
- additionally attempt Outprize PWM gap decoding

Expected successful Outprize output includes:

```text
===== OUTPRIZE_PACKET_CANDIDATE =====
Decoder: PWM gap short=0 long=1
Edges: 70  DecodeStartIndex: ...  Bits: ...
Binary: ...
Hex: ...
Low24: 0x......
====================================
```

## ESPHome YAML

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
