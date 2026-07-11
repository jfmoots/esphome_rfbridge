# RF Bridge Architecture

RF Bridge is an extensible RF transport appliance. It is not itself a fan, TPMS, light, or lock integration.

## Layers

```text
Home Assistant integrations
  - discovery and naming
  - device/entity registry
  - behavior policy
  - state presentation
            |
            v
Stable bridge API and decoded events
            |
            v
RF Bridge core
  - radio ownership
  - capture scheduling
  - event routing
  - capability reporting
            |
      +-----+-----+
      |           |
      v           v
Protocol codecs   Radio backends
  outprize:v1       cc1101
  tyreguard:future  stx882
  future codecs     srx882
```

## Codec responsibilities

A codec owns protocol-specific knowledge:

- recognition and validation
- packet decoding
- packet/state encoding
- waveform manufacture
- preferred normal RX backend
- preferred normal TX backend
- optional diagnostic backend

The first codec is Outprize v1. It declares:

```text
rx_backend: cc1101
tx_backend: stx882
diagnostic_rx_backend: srx882
```

A future TyreGuard codec will likely declare CC1101 receive and no transmit backend.

## Home Assistant responsibilities

Home Assistant integrations own:

- discovery workflows
- remote/sensor IDs and friendly names
- entities and devices
- assumed-state policy
- multi-device configuration
- user-facing diagnostics

The ESP does not create fan, cover, rain, tire, or other product-specific entities.

## Compatibility

v1.5.0 keeps the v1.4.1 Outprize API contract while moving toward codec-based discovery and events. This lets the HA integration be built before removing temporary test surfaces.
