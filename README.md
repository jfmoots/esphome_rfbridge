# ESPHome RF Bridge v1.4.0

v1.4.0 is the transport/API baseline for the Outprize Home Assistant integration.

The ESP32 owns RF reception, protocol decoding, requested-state caching, canonical waveform generation, and STX882 transmission. A separate Home Assistant integration will own the native fan, cover, rain configuration, and diagnostic entities.

Use `esphome/examples/rfbridge_outprize_bridge.yaml` as the matching YAML/API contract.
