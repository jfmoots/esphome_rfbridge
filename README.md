# ESPHome RF Bridge v1.3.12.1

ESPHome external component for the Outprize RF Bridge using ESP32 + CC1101.

v1.3.12 focuses on OEM capture learning and replay. After an OEM Outprize remote packet is captured and decoded, the component stores the recovered 35-bit frame and can replay that learned frame from a Home Assistant button.

## New v1.3.12 test helpers

```yaml
button:
  - platform: template
    name: "Outprize Replay Last Learned OEM Frame"
    on_press:
      - lambda: |-
          id(rf_bridge).replay_last_outprize_learned();

  - platform: template
    name: "Outprize Clear Learned OEM Frame"
    on_press:
      - lambda: |-
          id(rf_bridge).clear_last_outprize_learned();
```

## Test flow

1. Flash v1.3.12 with `diagnostic_logging: true`.
2. Open ESPHome logs.
3. Press the OEM Outprize remote button once, preferably Power Off.
4. Wait for a log line beginning with `OUTPRIZE_LEARNED`.
5. Press `Outprize Replay Last Learned OEM Frame` near the fan.

If no `OUTPRIZE_LEARNED` line appears, the ESP did not decode a clean enough OEM frame yet. Try pressing the OEM remote again from a different distance.


## v1.3.12 frequency trim helpers

Public lambda helpers added:

```cpp
id(rf_bridge).send_outprize_power_off_433900();
id(rf_bridge).send_outprize_power_off_433920();
id(rf_bridge).send_outprize_power_off_433940();
id(rf_bridge).send_outprize_power_off_433950();
id(rf_bridge).send_outprize_power_off_433970();
```

These transmit the normal Power Off frame with identical protocol timing while changing only the CC1101 TX frequency registers. RX restores to the normal 433.920 MHz profile afterward.
