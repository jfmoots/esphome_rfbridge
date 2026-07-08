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


## v1.1.0 Transmit Testing

This release adds the first experimental Outprize transmitter. The verified decoder remains the baseline; TX is intentionally simple and logs each send.

Default Outprize prefix: `0x6CF`

Example ESPHome template buttons:

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
    name: Outprize Fan 60 OUT
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize(
            0x6CF,
            60,
            esphome::rfbridge::OutprizeDirection::OUT,
            false,
            esphome::rfbridge::OutprizeVentCommand::NONE
          );

  - platform: template
    name: Outprize Fan Off Awake
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_fan_off(0x6CF);

  - platform: template
    name: Outprize Power Off
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_power_off(0x6CF);

  - platform: template
    name: Outprize Vent Open
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize(
            0x6CF,
            60,
            esphome::rfbridge::OutprizeDirection::OUT,
            false,
            esphome::rfbridge::OutprizeVentCommand::OPEN
          );
```

Recommended first test: use a harmless command such as FAN OFF / awake idle (`0x600040`) while watching the RF log, then test a remembered fan-state command such as 60% OUT (`0x600340`).
