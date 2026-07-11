# ESPHome RF Bridge v1.3.24

ESPHome external component for an ESP32 RF laboratory using:

- CC1101 for narrowband 433.92 MHz receive/analysis and experimental TX
- STX882 for direct ASK/OOK transmission
- SRX882/SRX882S for independent ASK/OOK raw capture

## v1.3.24 hardware mapping

- CC1101 GDO0: GPIO4
- CC1101 GDO2: GPIO27
- STX882 DATA: GPIO26
- SRX882 DATA: GPIO25
- SRX882 CS/enable: GPIO33 (HIGH = enabled)

See `esphome/examples/rfbridge_outprize_bridge.yaml` for a complete example.
