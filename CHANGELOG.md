# Changelog

## v1.5.0 — Codec Architecture Foundation

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
