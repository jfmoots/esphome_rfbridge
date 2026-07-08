# Changelog

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
