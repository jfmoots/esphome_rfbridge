# Changelog

## v1.3.26 — Smart RF Burst Recorder

- Replaces full-window SRX882 recording with burst extraction.
- Arms for up to the requested window but stores only the highest-quality RF burst.
- Splits acquisition data at long quiet gaps.
- Scores candidate bursts using edge density and expected ASK/OOK timing ranges.
- Rejects SRX882 idle chatter before and after the selected burst.
- Reports selected edge count, duration, and rejected edge counts in Home Assistant.
- Keeps the proven SRX882 capture and STX882 raw replay backend unchanged.
- No YAML changes from v1.3.25.
