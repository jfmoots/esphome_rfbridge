# Architecture

RF Bridge separates RF hardware, protocol knowledge, transport, and Home Assistant device behavior into independent layers.

## Layer model

```text
┌──────────────────────────────────────────────────────┐
│ Home Assistant integrations                          │
│ Discovery, config entries, entities, state, UX       │
└──────────────────────────┬───────────────────────────┘
                           │
             decoded events / addressed commands
                           │
┌──────────────────────────▼───────────────────────────┐
│ RF Bridge transport                                  │
│ Event routing, API actions, heartbeat, capabilities  │
└───────────────┬───────────────────────┬──────────────┘
                │                       │
┌───────────────▼──────────────┐  ┌────▼────────────────┐
│ Protocol codecs              │  │ Radio backends       │
│ Decode, encode, validate     │  │ Capture and transmit │
│ Outprize, future TyreGuard   │  │ CC1101/STX882/SRX882 │
└──────────────────────────────┘  └─────────────────────┘
```

## RF Bridge core

The core owns:

- ESPHome component lifecycle
- Radio initialization
- Interrupt and capture scheduling
- Backend routing
- Codec registration and capability reporting
- Home Assistant event transport
- ESPHome API action transport
- Bridge health, heartbeat, and boot/session identity

The core does not own friendly device names, areas, dashboards, or protocol-specific Home Assistant entities.

## Radio backends

Backends operate physical radios. They should expose capabilities rather than application behavior.

Current hardware roles:

- **CC1101:** normal receive path and optional asynchronous transmit path
- **STX882:** direct ASK/OOK transmit path
- **SRX882:** optional diagnostic/raw receive path

The bridge owns hardware access. A codec can declare which backend it requires or prefers, but the codec should not directly manipulate another bridge component through circular callbacks.

## Protocol codecs

A codec owns protocol knowledge:

- Frame recognition
- Bit order and field mapping
- Checksum or validity rules
- Remote/sensor identifier extraction
- Logical state decoding
- Logical command encoding
- Canonical waveform creation where needed
- Preferred receive/transmit backend declarations

A codec returns structured information to the bridge. It does not create Home Assistant entities.

## Home Assistant integrations

Integrations own the application model:

- Discover and name remotes or sensors
- Store remote/sensor identifiers
- Create native entities
- Persist assumed state
- Track availability
- Apply usability and safety policies
- Reconcile OEM transmissions with HA commands
- Present diagnostics in a device-friendly form

For Outprize, this means the integration decides that setting a fan speed should open the vent, that fan-off can leave the vent open, and that closing the vent should stop the fan first. These are product behaviors, not RF encoding rules.

## State and authority

Many inexpensive RF devices are one-way. RF Bridge and integrations therefore track the **last valid commanded state**, not confirmed physical telemetry.

For Outprize:

- Every OEM remote press carries a complete requested state.
- An OEM frame is authoritative for the requested state at that moment.
- HA commands update assumed state immediately.
- Restored state after HA restart is assumed and causes no automatic transmission.
- The next OEM frame or HA command replaces restored state.

## Discovery

RF Bridge emits every complete valid frame, including frames from unknown remote IDs. The Home Assistant integration decides whether to:

- Update an existing device
- Start a discovery flow
- Accept a manually requested pairing window
- Ignore a previously rejected remote

The ESPHome YAML does not need one hard-coded remote ID per fan.

## Availability and reconnect

The bridge emits a status heartbeat containing a random boot/session ID, firmware version, and capabilities.

The integration can distinguish:

- Normal continued operation
- Heartbeat loss
- Network interruption
- Bridge reboot (boot ID changes)
- Firmware or codec capability changes

An ESP reboot must not require rediscovery or re-pairing.

## Capability contract

A representative capabilities string is:

```text
bridge=rfbridge version=1.6.0 radios[cc1101_rx=YES,cc1101_tx=yes,stx882_tx=YES,srx882_rx=YES] codecs[outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882]
```

Integrations should verify the required codec before enabling controls.

## Why an ESP bridge instead of USB radios on the HA host

The ESP32 provides a dedicated real-time environment for:

- Interrupt-driven edge capture
- Microsecond waveform timing
- Tight radio control
- Deterministic ASK/OOK generation
- Local protocol filtering

It also allows the bridge to be placed at the best RF location rather than beside the Home Assistant computer. Home Assistant remains insulated from Linux USB enumeration, device permissions, custom daemons, and timing-sensitive GPIO work.
