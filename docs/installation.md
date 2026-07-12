# Installation

## Requirements

- ESPHome 2026.4.0 or newer
- ESP32-compatible board
- RF Bridge hardware matching the configured radio pins
- Home Assistant with the ESPHome integration
- A protocol-specific integration such as `ha_outprize`

## External component

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/jfmoots/esphome_rfbridge
      ref: v1.6.0
      path: esphome/components
    components:
      - rfbridge
```

Pinning to the release tag prevents later changes on `main` from affecting a known-good installation.

## Production configuration

Start from:

```text
esphome/examples/rfbridge_production.yaml
```

Merge its `api:` actions into the existing `api:` block. ESPHome permits only one top-level `api:` key.

Set every `bridge_id` value to the stable ESPHome node name used by the integration. In the reference installation:

```text
esphome-web-bafaac
```

## Validate and flash

Validate the YAML and perform a clean build when moving between major component structures.

Expected startup markers:

```text
Firmware Version: 1.6.0
Boot ID: <random 32-bit value>
CC1101 Detected: YES
STX882 Available: YES
SRX882 Available: YES
Codec: outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882
```

## Home Assistant permissions

ESPHome must be allowed to perform Home Assistant actions for the `homeassistant.event` transport blocks to publish events.

## Install the Outprize integration

1. Copy `custom_components/outprize` from the `ha_outprize` release into `/config/custom_components/`.
2. Restart Home Assistant.
3. Add **Outprize Fan** from **Settings → Devices & services**.
4. Select or enter the bridge ID.
5. Press any button on the desired OEM remote.
6. Name the fan and assign an area.

The integration creates native entities for:

- Fan power, speed, and direction
- Vent up/stop/down control
- Rain-sensor enable setting

## Multiple fans

Each OEM remote ID becomes a separate Home Assistant device. One centrally located RF Bridge can receive and command multiple Outprize fans.

No remote IDs need to be hard-coded in production ESPHome YAML.

## Expected ESPHome device appearance

A production bridge may show no entities on its Home Assistant device page. This is expected. The protocol-specific integration owns user-facing entities.

## Updating

1. Update the repository/tag reference.
2. Review release notes for YAML contract changes.
3. Validate and flash the bridge.
4. Update the Home Assistant integration if required.
5. Verify heartbeat, remote reception, and one addressed transmit command.

The boot ID changes after every bridge reboot, allowing integrations to recover without re-pairing.
