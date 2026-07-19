# RF Bridge

**RF Bridge is an extensible RF middleware platform for ESPHome and Home Assistant.**

It connects low-cost RF hardware to protocol-specific Home Assistant integrations without turning the ESP into the user interface. The bridge owns radio timing, waveform handling, protocol codecs, transport, health, and capability reporting. Home Assistant integrations own discovery, configuration, device state, entities, and user experience.

The result is intentionally quiet: a production RF Bridge may expose no normal Home Assistant entities at all while still receiving, decoding, routing, and transmitting RF traffic.

## What it does

RF Bridge provides four independent layers:

1. **Radio backends** capture and transmit RF waveforms.
2. **Protocol codecs** translate between waveforms and structured protocol data.
3. **Bridge transport** publishes decoded events and accepts addressed commands.
4. **Home Assistant integrations** turn those events and commands into native devices and entities.

```text
Home Assistant integrations
        │
        │ decoded events / addressed commands
        ▼
RF Bridge transport
        │
        ├── Protocol codecs
        │     └── Outprize v1
        │
        └── Radio backends
              ├── CC1101 RX / optional TX
              ├── STX882 ASK/OOK TX
              └── SRX882 diagnostic RX
```

## Current capabilities

### Radio hardware

- **CC1101** — primary configurable receiver; optional asynchronous transmitter
- **STX882** — direct ASK/OOK transmitter for protocols that benefit from a simple envelope path
- **SRX882** — optional diagnostic/raw-capture receiver
- **ESP32 DevKit** — proven host platform

### Bridge services

- Event-driven decoded frame delivery
- Addressed protocol commands
- Runtime capability advertisement
- Firmware and codec version reporting
- Random boot/session identifier
- Periodic bridge heartbeat
- Automatic recovery support for Home Assistant integrations
- Raw capture, replay, and analyzer foundations retained for development work

### Included codec

#### Outprize v1

Production support for Outprize RV vent-fan remotes:

- Full-state remote decoding
- Remote-ID discovery
- Ten-speed Gray-coded fan control
- Intake/exhaust direction
- Rain-sensor setting
- Vent up, stop, and down commands
- Fan-off/awake behavior independent of vent position
- Canonical ASK/OOK waveform generation
- CC1101 reception and STX882 transmission
- Native Home Assistant support through `ha_outprize`

## Design principles

### RF Bridge owns radios

The bridge handles interrupts, RSSI gating, edge timing, waveform generation, radio configuration, and backend routing.

### Codecs own protocols

A codec understands packet structure, bit mappings, checksums, state fields, and waveform encoding. A codec does not create Home Assistant entities and does not directly own the radios.

### Integrations own devices

Home Assistant integrations handle pairing, naming, areas, persistence, availability, assumed state, behavior policy, and native entities.

### Production firmware stays humble

Protocol-specific diagnostic entities are not part of the normal production configuration. The bridge reports structured events and health information, then remains quiet until work is required.

## Hardware

The proven reference build uses:

| Function | Hardware | ESP32 connection |
|---|---|---|
| CC1101 chip select | CC1101 CS | GPIO5 |
| SPI clock | CC1101 SCK | GPIO18 |
| SPI MOSI | CC1101 MOSI | GPIO23 |
| SPI MISO | CC1101 MISO | GPIO19 |
| CC1101 data | GDO0 | GPIO4 |
| CC1101 auxiliary data | GDO2 | GPIO27 |
| Direct transmitter | STX882 DATA | GPIO26 |
| Diagnostic receiver | SRX882 DATA | GPIO25 |
| Diagnostic receiver enable | SRX882 CS/EN | GPIO33 |

All RF modules use **3.3 V logic** and must share ground with the ESP32. Verify the pin labels on the exact modules in hand; inexpensive RF boards are not always laid out consistently.

For 433.92 MHz, a straight quarter-wave wire is approximately **17.3 cm / 6.8 in**, measured electrically from the antenna feed point. Central placement, vertical antenna orientation, and reasonable separation between transmit and receive antennas generally produce the best results.

See [Hardware Guide](docs/hardware.md) for the complete reference build and installation notes.

## Installation

### 1. Add the external component

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/jfmoots/esphome_rfbridge
      ref: main
      path: esphome/components
    components:
      - rfbridge
    refresh: 0s
```

For reproducible production deployments, pin `ref` to a release tag such as `v1.6.0` rather than `main`.

### 2. Configure the bridge hardware and transport

A complete production example is included at:

```text
esphome/examples/rfbridge_production.yaml
```

The current production transport uses two Home Assistant events and two ESPHome API actions for the Outprize codec. These are transport adapters, not user-facing entities.

### 3. Flash the ESP32

Validate and install through ESPHome. On startup, the log should show:

```text
Firmware Version: 1.6.0
CC1101 Detected: YES
STX882 Available: YES
Codec: outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882
```

### 4. Install the protocol-specific Home Assistant integration

For Outprize fans, install `ha_outprize`, add the integration, and press any button on the OEM remote when prompted. The integration discovers the remote ID and creates the native fan, vent, and rain entities.

The RF Bridge device itself is expected to look nearly empty in Home Assistant. That is intentional.

## Home Assistant transport contract

### Events

- `esphome.rfbridge_outprize_frame` — every complete valid Outprize frame
- `esphome.rfbridge_status` — firmware, capabilities, boot ID, and heartbeat

### Actions

- `outprize_send_complete_state` — addressed full-state transmission
- `outprize_send_fan_off_awake` — addressed fan-off command with independent vent action
- `outprize_send_complete_state_family` — variant-aware addressed full-state transmission
- `outprize_send_fan_off_awake_family` — variant-aware fan-off and vent action

See [API Reference](docs/api.md) for field definitions and behavioral guarantees.

## Adding another protocol

A new protocol is added as a codec, not as another monolithic ESP project.

Typical work:

1. Add a codec that recognizes and decodes captures.
2. Add encoding support if the protocol transmits.
3. Declare the preferred radio backends.
4. Advertise the codec in bridge capabilities.
5. Publish structured events and accept addressed commands.
6. Build a protocol-specific Home Assistant integration.

A receive-only TyreGuard TPMS codec is a natural next example: RF Bridge would decode sensor ID, pressure, temperature, and status; a Home Assistant integration would own tire assignment, units, thresholds, stale-data handling, and dashboard entities.

See [Codec Development](docs/codec_development.md).

## Repository layout

```text
README.md
CHANGELOG.md

docs/
  architecture.md
  hardware.md
  installation.md
  api.md
  codec_development.md
  reverse_engineering.md
  repository_layout.md

esphome/
  components/
    rfbridge/
      __init__.py
      rfbridge.h
      rfbridge.cpp
      codec.h
      outprize_codec.h
      outprize_codec.cpp
      cc1101_regs.h
      version.h
  examples/
    rfbridge_production.yaml
```

## Development and diagnostics

The production configuration keeps `diagnostic_logging: false` and omits the old high-frequency template sensors. Raw capture, waveform replay, timing analysis, and codec-development tooling remain valuable, but they are development facilities rather than the normal Home Assistant interface.

A dedicated developer mode or developer firmware is a future enhancement. Until then, keep production YAML small and add diagnostic controls only while actively investigating a protocol.

## Project status

RF Bridge v1.6.0 is the first platform-oriented release:

- The bridge/core boundary is established.
- Outprize is packaged as the first production codec.
- `ha_outprize` supplies the user-facing Home Assistant device model.
- Multi-remote addressed transport and discovery are proven.
- Bridge heartbeat and session-aware reconnection are supported.
- The production bridge exposes no protocol-specific state entities.

The architecture is designed to grow by adding codecs and integrations rather than by turning RF Bridge into a larger application.

## Documentation

- [Architecture](docs/architecture.md)
- [Hardware Guide](docs/hardware.md)
- [Installation](docs/installation.md)
- [API Reference](docs/api.md)
- [Codec Development](docs/codec_development.md)
- [Repository Layout](docs/repository_layout.md)
- [Outprize Reverse-Engineering Notes](docs/reverse_engineering.md)
