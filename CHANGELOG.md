# Changelog

## v0.10.2 – Protocol Dispatcher Cleanup

- Keeps the verified Outprize protocol model unchanged.
- Makes the normal receive path decode-first and protocol-oriented.
- Suppresses raw timings, histograms, symbol streams, RLE, and motif analysis unless `diagnostic_logging: true`.
- Logs successful Outprize captures as compact `OUTPRIZE low24=... confidence=...` events.
- Keeps multi-alignment candidate scoring from v0.10.1.
- Adds human-friendly confidence labels derived from the internal score.
- Treats unknown/non-Outprize captures as cheap ignored events in normal mode.
- Documents protocol-dispatcher direction for future candidates such as TyreGuard.
- Documents that RVLock rolling-code RF decoding is out of scope; that project will use a physical remote/button-push path.

## v0.10.1 – Decoder Robustness and Lean Logging

- Keeps the verified Outprize protocol model unchanged.
- Promotes normal receive path to lean decode-first logging.
- Adds optional `diagnostic_logging` for raw timings, histograms, symbols, and analyzer output.
- Scans multiple plausible PWM alignments and scores decoded candidates.
- Prefers verified `0x60xxxx` Outprize payload family when selecting the best decode.
- Logs Outprize-like captures that do not decode as `candidate_no_decode`.

## v0.10.0 – Outprize Candidate Decoder

- Added first Outprize-specific decoder output.
- Detects Outprize-like PWM gap captures.
- Decodes short gap = 0 and long gap = 1.
- Logs binary, grouped hex, and Low24 values.
- Keeps v0.9.0 fingerprint/symbol analyzer output.
