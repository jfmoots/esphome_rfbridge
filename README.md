# ESPHome RF Bridge

ESPHome external component for a CC1101-based RF bridge used in the MooterHome Outprize vent fan reverse-engineering project.

## v1.1.1 focus

This release fixes the ESPHome automation interface for the first Outprize transmitter. v1.1.0 added the C++ transmit helpers, but the YAML action `rfbridge.send_outprize_low24` was not registered with ESPHome. v1.1.1 registers the transmit actions so template buttons can call TX directly from YAML.

The verified v1.0.0 receive decoder and verified Outprize protocol model are unchanged. RVLock remains intentionally out of scope for RF rolling-code decoding; that control path will use a cannibalized physical remote/button-push approach instead.

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


## v1.1.1 Transmit Testing

Default Outprize prefix / remote ID: `0x6CF`

The component now exposes ESPHome automation actions:

- `rfbridge.send_outprize_low24`
- `rfbridge.send_outprize_fan_off`
- `rfbridge.send_outprize_power_off`

Because ESPHome actions need to know which component instance to call, give the `rfbridge` an `id` and include that same id in each action.

```yaml
rfbridge:
  id: rf_bridge
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
  diagnostic_logging: false

button:
  - platform: template
    name: Outprize TX Test 40 Percent OUT
    on_press:
      - rfbridge.send_outprize_low24:
          id: rf_bridge
          low24: 0x600140

  - platform: template
    name: Outprize TX Test Fan Off Awake
    on_press:
      - rfbridge.send_outprize_fan_off:
          id: rf_bridge

  - platform: template
    name: Outprize TX Test Power Off
    on_press:
      - rfbridge.send_outprize_power_off:
          id: rf_bridge

  - platform: template
    name: Outprize TX Test 60 Percent OUT
    on_press:
      - rfbridge.send_outprize_low24:
          id: rf_bridge
          low24: 0x600340
          repeats: 3
```

Recommended first test: use FAN OFF / awake idle (`0x600040`) while watching the RF log, then test a remembered fan-state command such as 40% OUT (`0x600140`) or 60% OUT (`0x600340`).
