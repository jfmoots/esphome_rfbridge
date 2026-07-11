# RF Bridge v1.5.5

RF Bridge is an extensible ESPHome RF middleware component. Protocol codecs decode and encode RF protocols while the bridge owns the radios, routing, diagnostics, and transport.

## Installed codec

- `outprize:v1`
  - RX: CC1101
  - TX: STX882
  - Diagnostic RX: SRX882

## New in v1.5.5

The bridge now emits every complete, valid Outprize frame through `on_outprize_frame`, regardless of the remote ID configured for the temporary single-remote diagnostic cache. This provides the discovery/event path required by `ha_outprize`.

The bridge also provides a stateless addressed transmit method so a Home Assistant integration can send a full state to any discovered Outprize remote ID without changing ESPHome YAML.

## RF Bridge configuration

```yaml
rfbridge:
  id: rf_bridge
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
  gdo2_pin: GPIO27
  stx882_data_pin: GPIO26
  srx882_data_pin: GPIO25
  srx882_enable_pin: GPIO33
  diagnostic_logging: false

  on_outprize_frame:
    then:
      - homeassistant.event:
          event: esphome.rfbridge_outprize_frame
          data:
            bridge_id: esphome-web-bafaac
          data_template:
            remote_id: "{{ rf_remote_id }}"
            low24: "{{ rf_low24 }}"
            powered: "{{ rf_powered }}"
            speed_percent: "{{ rf_speed_percent }}"
            direction_in: "{{ rf_direction_in }}"
            rain_enabled: "{{ rf_rain_enabled }}"
            vent_command: "{{ rf_vent_command }}"
            rssi_dbm: "{{ rf_rssi_dbm }}"
          variables:
            rf_remote_id: !lambda return remote_id;
            rf_low24: !lambda return low24;
            rf_powered: !lambda return powered;
            rf_speed_percent: !lambda return speed_percent;
            rf_direction_in: !lambda return direction_in;
            rf_rain_enabled: !lambda return rain_enabled;
            rf_vent_command: !lambda return vent_command;
            rf_rssi_dbm: !lambda return rssi_dbm;
```

Home Assistant must allow this ESPHome device to perform Home Assistant actions so it can fire the event.

## Addressed API action

Merge this action into the existing `api: actions:` list:

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

The existing v1.4.1/v1.5.4 diagnostic actions and sensors remain compatible.
