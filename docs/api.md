# API Reference

RF Bridge v1.6.0 currently uses ESPHome Home Assistant events for decoded/status delivery and ESPHome API actions for commands.

These adapters are intentionally protocol-specific for the current release. A future transport may hide more of this YAML without changing the architecture described here.

## Event: `esphome.rfbridge_outprize_frame`

Emitted for every complete valid Outprize frame, regardless of whether the remote ID is already configured in Home Assistant.

Fields:

| Field | Type | Description |
|---|---|---|
| `bridge_id` | string | Stable bridge identifier |
| `remote_id` | integer | 11-bit Outprize remote prefix |
| `low24` | integer | Decoded 24-bit command/state field |
| `powered` | bool/int | Requested fan power state |
| `speed_percent` | integer | Decoded speed, 10 through 100 |
| `direction_in` | bool/int | `true` for intake/IN, `false` for exhaust/OUT |
| `rain_enabled` | bool/int | Rain-sensor setting |
| `vent_command` | integer | `0` none, `4` down/close, `8` up/open, `12` stop |
| `rssi_dbm` | integer | Receive RSSI at capture time |

Example:

```yaml
bridge_id: esphome-web-bafaac
remote_id: 1743
low24: 6292816
powered: true
speed_percent: 90
direction_in: false
rain_enabled: true
vent_command: 0
rssi_dbm: -54
```

`1743` decimal is remote ID `0x6CF`.

## Event: `esphome.rfbridge_status`

Published periodically as the bridge heartbeat.

Fields:

| Field | Type | Description |
|---|---|---|
| `bridge_id` | string | Stable bridge identifier |
| `boot_id` | integer | Random 32-bit value generated on each boot |
| `firmware_version` | string | RF Bridge firmware version |
| `capabilities` | string | Installed radios, codecs, and preferred backends |

Integrations use heartbeat expiry to mark devices unavailable and use boot-ID changes to identify bridge restarts.

## Action: `outprize_send_complete_state`

Sends a complete addressed Outprize state.

Input fields:

| Field | Type | Description |
|---|---|---|
| `remote_id` | integer | Target 11-bit remote ID |
| `powered` | bool | Requested power state |
| `speed_percent` | integer | Desired speed; normalized to supported 10% steps |
| `direction_in` | bool | Intake when true, exhaust when false |
| `rain_enabled` | bool | Rain-sensor setting |
| `vent_command` | integer | `0`, `4`, `8`, or `12` |

The bridge encodes the protocol state and transmits the canonical waveform through the STX882 backend.

## Action: `outprize_send_fan_off_awake`

Stops the fan motor without invoking the dedicated full-power-off behavior that closes the vent.

Input fields:

| Field | Type | Description |
|---|---|---|
| `remote_id` | integer | Target 11-bit remote ID |
| `vent_command` | integer | Optional independent vent action |

This action supports:

- Fan off, vent unchanged: `vent_command: 0`
- Fan off, vent down: `vent_command: 4`
- Fan off, vent up: `vent_command: 8`
- Fan off, stop vent: `vent_command: 12`

## Behavioral guarantees

- Commands are addressed by remote ID.
- The Outprize codec performs protocol encoding.
- The bridge selects the codec-declared STX882 transmit backend.
- A command does not require the remote ID to exist in an ESP-side registry.
- HA restart or restored state does not automatically transmit RF.
- OEM remote frames can update integration state independently of HA-originated commands.

## Compatibility

`ha_outprize` v0.1.3 expects:

- Frame event support introduced in RF Bridge v1.5.5
- Working addressed STX882 transmission from v1.5.6
- Fan-off/awake support from v1.5.7
- Bridge heartbeat and boot ID from v1.5.8
- Clean-build-safe codec packaging from v1.5.9

RF Bridge v1.6.0 includes all of the above.
