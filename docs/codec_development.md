# Codec Development

A codec packages protocol knowledge so RF Bridge can remain a general radio and transport platform.

## Codec responsibilities

A codec should own:

- Protocol identification
- Frame validation
- Bit and field decoding
- Address/remote/sensor identifier extraction
- Logical state representation
- Command encoding
- Canonical waveform generation, where required
- Preferred receive/transmit backend declarations
- Capability versioning

A codec should not own:

- Home Assistant entity classes
- Friendly names or areas
- User configuration flows
- Dashboard logic
- Persistent HA state
- Direct circular calls back into `RFBridgeComponent`

## Pure boundary

The intended flow is:

```text
RF capture
   ↓
codec.decode(capture)
   ↓
decoded protocol event
   ↓
RF Bridge transport
```

and:

```text
logical protocol command
   ↓
codec.encode(command)
   ↓
encoded frame / waveform + backend preference
   ↓
RF Bridge backend router
```

The bridge owns radio execution. The codec returns protocol results.

## Versioning

Each codec should advertise a stable ID and version, for example:

```text
outprize:v1
```

Breaking changes to event or command semantics require a codec version increase.

## Adding a receive-only codec

A TyreGuard TPMS codec would typically:

1. Configure or request a compatible CC1101 receive profile.
2. Recognize the modulation/timing pattern.
3. Validate packet integrity.
4. Decode sensor ID, pressure, temperature, and battery/status fields.
5. Emit a structured event.
6. Advertise `tx=none`.

The Home Assistant integration would handle tire-position assignment, units, alerts, stale-data timers, and native sensors.

## Adding a transmit codec

A transmit-capable codec should:

1. Define a logical command structure.
2. Validate and normalize requested fields.
3. Encode the protocol payload.
4. Build the required waveform or packet representation.
5. Declare the preferred backend.
6. Return the encoded result to the bridge for transmission.

## Backend selection

The codec may declare:

```text
rx=cc1101
tx=stx882
diagnostic_rx=srx882
```

This declaration is capability metadata. The bridge still owns and operates the hardware.

## Event design

Events should be complete enough that a Home Assistant integration does not need RF knowledge.

Good event fields:

- Protocol ID/version
- Device/remote/sensor ID
- Decoded logical values
- Receive metadata such as RSSI
- Optional raw payload for diagnostics

Avoid requiring the integration to decode Gray code, checksums, bit fields, or waveform timing.

## Command design

Commands should be addressed and logical:

```text
protocol=outprize
remote_id=0x6CF
power=true
speed=50
direction=out
rain=true
vent=none
```

Avoid exposing raw edge arrays as the normal product interface.

## Testing

A codec should be tested against:

- Known valid captures
- Invalid/noisy captures
- Boundary values
- Multiple device identifiers
- Round-trip encode/decode where applicable
- Clean ESPHome builds
- Incremental ESPHome builds
- Capability output
- Hardware receive/transmit behavior

Keep reverse-engineering captures and reasoning in protocol-specific documentation so future changes do not require rediscovery.
