# Architecture

`esphome_rfbridge` is a transport appliance, not a fan/light/lock integration.

## Responsibilities

The ESPHome RF bridge should:

- own the CC1101 radio
- transmit RF packets
- receive RF packets
- decode packets into protocol-level events when possible
- support remote ID learning
- expose low-level services/actions to Home Assistant

The ESPHome RF bridge should not:

- create fan entities
- create lock entities
- create light entities
- model Home Assistant device state
- decide what speed, direction, or vent state means to a user

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
  rvlock
  trimark
  boogey_lights
```

Outprize is the first protocol because we have a complete packet model.
