# Architecture

`esphome_rfbridge` is a transport appliance and protocol bridge, not a fan/light/lock integration.

## Responsibilities

The ESPHome RF bridge should:

- own the CC1101 radio
- transmit RF packets
- receive RF packets
- cheaply classify captures
- run candidate protocol decoders when appropriate
- decode packets into protocol-level events when possible
- support remote ID learning
- expose low-level services/actions to Home Assistant

The ESPHome RF bridge should not:

- create fan entities
- create lock entities
- create light entities
- model Home Assistant device state
- decide what speed, direction, or vent state means to a user

## Receive pipeline

```text
capture
  -> cheap classifier
  -> candidate decoders
       - Outprize
       - TyreGuard (future)
       - other OOK protocols (future)
  -> optional diagnostic analyzer
```

Normal mode should remain fast and quiet. Full raw timings, histograms, symbol streams, RLE, and motifs are diagnostic tools only.

## Layering

```text
Home Assistant integration
        │
        ▼
ESPHome native API action/service
        │
        ▼
esphome_rfbridge
        │
        ▼
CC1101
        │
        ▼
433 MHz device
```

## Protocol plugins

Protocols should be isolated behind encode/decode helpers. The bridge can eventually register multiple protocol implementations:

```text
protocols/
  outprize
  tyreguard
  other_ook
```

Outprize is the first protocol because we have a complete packet model. TyreGuard is a plausible future receive-only candidate. RVLock rolling-code RF decoding is not planned for this bridge; that project will use a cannibalized physical remote/button-push path instead.
