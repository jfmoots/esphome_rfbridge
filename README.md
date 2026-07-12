# ESPHome RF Bridge v1.5.8

v1.5.8 adds the bridge lifecycle contract used by Home Assistant integrations.
The bridge now publishes a status heartbeat every 10 seconds containing a unique
boot/session ID, firmware version, and advertised radios/codecs.

Normal Outprize operation remains:

- RX: CC1101
- TX: STX882
- Diagnostic RX: SRX882
- Codec: `outprize:v1`

The ESP remains a transport appliance. Fan, vent, rain, discovery, persistence,
and user-facing entities belong to `ha_outprize`.

## Required production YAML

Keep the existing addressed Outprize actions. Add `on_bridge_status` beside the
existing `on_outprize_frame` trigger inside the same `rfbridge:` block.
See `esphome/examples/rfbridge_production.yaml` for the complete matching
transport section.

The status event is a heartbeat. `ha_outprize` v0.1.3 marks its entities
unavailable if status stops arriving, and automatically recovers when the bridge
returns with the same or a new boot ID.
