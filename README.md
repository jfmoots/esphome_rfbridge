# ESPHome RF Bridge v1.5.3

v1.5.3 fixes ESPHome codec linking while preserving the codec-oriented RF Bridge architecture and the proven v1.4.1 Outprize API/RF behavior.

The Outprize codec remains a separate class and interface, but its implementation is included in the primary `rfbridge.cpp` translation unit because ESPHome 2026.6.x does not automatically compile additional `.cpp` files shipped beside an external component.

## Architecture

- **Bridge core:** radio ownership, capture timing, backend availability, capability reporting.
- **Radio backends:** CC1101 RX/TX, STX882 TX, SRX882 diagnostic RX.
- **Protocol codecs:** protocol recognition, decode, state encoding, waveform generation, and preferred backend declaration.
- **Home Assistant integrations:** discovery, device naming, entities, policy, and presentation.

The first registered codec is `outprize:v1`:

- normal RX backend: `cc1101`
- normal TX backend: `stx882`
- diagnostic RX backend: `srx882`

No user-facing fan, cover, or switch entities are created by the component. The v1.4.1 API remains available so the future HA Outprize integration can be developed against a stable, proven transport contract.

Use `esphome/examples/rfbridge_outprize_bridge.yaml` as the matching YAML/API test configuration.
