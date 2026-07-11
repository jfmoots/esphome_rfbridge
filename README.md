# ESPHome RF Bridge v1.5.7

## Addressed STX882 routing fix

v1.5.7 fixes the addressed Outprize command path introduced in v1.5.5. The
multi-remote `outprize_send_complete_state` API now uses the same canonical
waveform generator and discrete STX882 transmitter path already proven by the
single-remote API.

The supplied remote ID is embedded in the 35-bit Outprize frame; it no longer
falls through to the legacy CC1101 asynchronous transmit path.

## YAML

No YAML changes are required from v1.5.5. Keep the existing
`on_outprize_frame` trigger and the existing addressed API action:

```yaml
    - action: outprize_send_complete_state
      variables:
        remote_id: int
        powered: bool
        speed_percent: int
        direction_in: bool
        rain_enabled: bool
        vent_command: int
      then:
        - lambda: |-
            id(rf_bridge).send_outprize_complete_state(
              static_cast<uint32_t>(remote_id),
              powered,
              static_cast<uint8_t>(speed_percent),
              direction_in
                ? esphome::rfbridge::OutprizeDirection::IN
                : esphome::rfbridge::OutprizeDirection::OUT,
              rain_enabled,
              static_cast<esphome::rfbridge::OutprizeVentCommand>(vent_command & 0x0C),
              1);
```

## Direct test

```yaml
action: esphome.esphome_web_bafaac_outprize_send_complete_state
data:
  remote_id: 1743
  powered: true
  speed_percent: 40
  direction_in: false
  rain_enabled: false
  vent_command: 8
```

Expected log markers include `codec=outprize`, `tx_backend=stx882`, and
`OUTPRIZE_BUILTIN TX full35=0x6CF...`.


## Addressed fan-off/awake API

Use `send_outprize_fan_off(remote_id, vent_command, repeats)` to stop the fan without invoking dedicated power-off. The vent action nibble may be NONE (0), CLOSE (4), OPEN (8), or STOP (12).
