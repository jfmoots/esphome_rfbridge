
## v1.3.28 built-in Outprize generator

Normal Outprize transmission no longer requires a learned SRX882 waveform. The firmware ships with the canonical accepted envelope and manufactures every 35-bit full-state command directly through the STX882. The recorder and analyzer remain available for diagnostics and future RF protocols.

# ESPHome RF Bridge v1.3.25

ESPHome external component for an ESP32 RF laboratory using:

- CC1101 for narrowband 433.92 MHz receive/analysis and experimental TX
- STX882 for direct ASK/OOK transmission
- SRX882/SRX882S for independent raw ASK/OOK recording

## RF Recorder

v1.3.25 promotes the proven SRX882-to-STX882 raw path into an explicit recorder:

1. Press **RF Recorder - Record Command**.
2. Press the OEM remote during the three-second recording window.
3. Confirm **RF Recorder Status** and **RF Recorded Command** show a valid recording.
4. Press **RF Recorder - Replay Command**.

The recorder is gated: ambient RF does not overwrite a recording unless the Record button is deliberately pressed.

## Hardware mapping

- CC1101 GDO0: GPIO4
- CC1101 GDO2: GPIO27
- STX882 DATA: GPIO26
- SRX882 DATA: GPIO25
- SRX882 CS/enable: GPIO33 (HIGH = enabled)

See `esphome/examples/rfbridge_outprize_bridge.yaml` for the complete matching YAML.
