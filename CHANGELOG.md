# Changelog

## v1.5.4 — Clean Codec Cache Compatibility Fix

- Fixes the v1.5.3 compile failure caused by ESPHome retaining the obsolete `outprize_codec_impl.h` from v1.5.2 in its external-component cache.
- Ships an intentional no-op compatibility shim at that path so stale cached source is overwritten safely.
- Removes all duplicate codec method definitions and all obsolete bridge-callback methods.
- Keeps `OutprizeCodec` as a pure protocol codec with no `RFBridgeComponent` pointer and no hardware I/O.
- Preserves the proven v1.4.1 Outprize receive, state API, canonical waveform generation, and STX882 transmit behavior.
- No YAML changes are required.
