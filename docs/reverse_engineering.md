# Outprize Reverse-Engineering Notes

This document preserves the protocol conclusions that underpin the production codec. It is a technical reference, not an installation guide.

## Device class

Outprize RV vent fan with a one-way 433.92 MHz ASK/OOK remote.

## Addressing

The protocol uses an 11-bit remote prefix followed by a 24-bit command/state field.

Reference remote:

```text
Remote prefix: 0x6CF
```

A complete logical value is therefore represented as a 35-bit frame such as:

```text
0x6CF600440
```

## Full-state behavior

Normal remote commands represent the requested complete fan state rather than simple `speed up` or `speed down` events. This allows one valid OEM frame to resynchronize Home Assistant state.

Decoded fields include:

- Power/fan behavior
- Ten-speed value
- Direction
- Rain-sensor setting
- Vent action

## Speed mapping

The ten speed levels use a Gray-coded table rather than a linear binary count. The codec owns this mapping. Home Assistant uses 10% steps from 10 through 100 and does not need to understand the bit representation.

## Direction and rain

Canonical field behavior discovered during capture analysis:

- Direction contributes `+0x20`
- Rain contributes `+0x10`

The codec treats these as logical fields and keeps bit-level details out of the integration.

## Vent commands

The vent command nibble is transient:

| Value | Meaning |
|---:|---|
| `0x00` | No vent action |
| `0x04` | Down / close |
| `0x08` | Up / open |
| `0x0C` | Stop |

Vent position is not reported by the fan. Home Assistant therefore exposes an assumed-state cover with Up, Stop, and Down controls.

## Special power behavior

Two distinct off behaviors are important:

### Dedicated power off

```text
Low24: 0x600000
```

This commands the OEM shutdown sequence: close the vent and stop the fan.

### Fan off / awake

```text
Low24 family: 0x600040 plus applicable vent action
```

This stops the fan motor without necessarily closing the vent. The Home Assistant integration uses this for normal fan-off and for independent vent control.

## Canonical waveform

The accepted manufactured waveform uses:

- 35 data bits
- MSB-first normal ordering
- Fixed leading/header structure
- Approximately 500 µs active pulse
- Approximately 500 µs gap for zero
- Approximately 1500 µs gap for one

Production timing is deterministic and no longer depends on a learned capture.

## Radio findings

### CC1101 receive

CC1101 reception proved reliable and provides useful RSSI, configurable demodulation, and interrupt-driven capture.

### CC1101 transmit

Multiple asynchronous OOK transmit experiments did not produce an envelope the target fan accepted reliably, despite successful protocol decoding and waveform analysis.

### SRX882/STX882 breakthrough

- SRX882 raw capture established an accepted waveform reference.
- STX882 replay of that capture controlled the fan.
- Modifying the captured data demonstrated that the decoded protocol was correct.
- A fixed canonical waveform manufacturer then removed the need for runtime learning or capture storage.

Current production roles:

```text
RX: CC1101
TX: STX882
Diagnostic RX: SRX882
```

## State semantics learned during integration testing

- Setting fan speed after a dedicated power-off does not automatically raise the vent at the protocol level.
- The Home Assistant integration adds the usability policy of opening the vent when starting the fan.
- Closing the vent while requesting fan-on can cause stop/restart/stop behavior as the fan controller enforces its local interlock.
- The integration therefore stops the fan before lowering the vent.
- A request to set fan speed to zero should not be treated as dedicated power-off because a user may want the vent left open.

## Known receive limitation

The current receiver uses an RSSI-gated fixed capture window. Very rapid successive remote presses can occasionally produce partial captures or miss a command. Accepted complete frames decode reliably. Improving capture recovery and buffering remains future bridge work.

## Why these notes are retained

These conclusions are the canonical protocol specification for future development. They should be used as the baseline rather than rediscovered from raw captures.
