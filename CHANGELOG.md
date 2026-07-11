# Changelog

## v1.5.3 — Pure Codec Boundary Fix

- Rebuilds `OutprizeCodec` as a pure protocol library with no pointer or callback into `RFBridgeComponent`.
- Removes the circular Bridge → Codec → Bridge dependency that caused the v1.5.2 compile failure.
- Moves Outprize direction and vent command types into the codec module.
- Moves speed normalization, Gray-code speed mapping, Low24 encoding, and speed decoding into the codec.
- Keeps radio capture, transmit routing, state caching, diagnostics, and ESPHome APIs in the bridge layer.
- Keeps the proven CC1101 receive and STX882 transmit behavior unchanged.
- No YAML changes from v1.5.0.


## v1.5.2 – ESPHome Codec Link Fix

- Fixes linker errors for `OutprizeCodec::capability_summary()` and the `OutprizeCodec` vtable.
- Keeps `OutprizeCodec` compartmentalized behind the generic `RFBridgeCodec` interface.
- Moves codec method definitions into an explicitly included implementation header so ESPHome compiles them in the primary `rfbridge.cpp` translation unit.
- Removes the uncompiled standalone `outprize_codec.cpp`.
- Preserves all v1.4.1 API, receive, decode, state-cache, and STX882 transmit behavior.
- No YAML changes from v1.5.0/v1.5.1.


## v1.5.1 – ESPHome Codec Source Packaging Fix

- Fixes ESPHome compilation failure caused by nested codec source files not being copied into the generated component source tree.
- Moves the Outprize codec implementation to the RF Bridge component root while preserving the codec abstraction and behavior.
- Updates include paths so `outprize_codec.h` and `outprize_codec.cpp` are compiled by ESPHome.
- No YAML or runtime behavior changes from v1.5.0.

## v1.5.1 — Codec Architecture Foundation

- Adds a generic `RFBridgeCodec` interface.
- Adds the first compartmentalized codec module: `codecs/outprize`.
- Declares codec identity, version, normal RX backend, normal TX backend, and diagnostic RX backend.
- Registers the Outprize codec with the RF Bridge core.
- Adds bridge capability reporting for radios and codecs.
- Keeps the proven CC1101 receive, STX882 transmit, SRX882 recorder, canonical Outprize manufacturer, state cache, and API behavior unchanged.
- Keeps v1.4.1 API actions and temporary test sensors compatible.
- Establishes the directory and interface pattern for future codecs such as TyreGuard.
- Creates no native fan, cover, switch, or device entities on the ESP.

## v1.4.1

- Corrected complete-state API test semantics.
- Added exact Low24 manufacture logging.
- Documented vent action nibble values.
