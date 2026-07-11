# Changelog

## v1.3.28 — Built-in Outprize Waveform Manufacturer

- Packages the accepted Outprize waveform structure directly in firmware.
- Removes the requirement to record or analyze an OEM command before normal transmission.
- Generates the full 35-bit frame MSB-first using remote prefix `0x6CF`.
- Uses the learned canonical envelope: 8 fixed header edges, 500 µs data pulses, 500 µs zero gaps, and 1500 µs one gaps.
- Keeps the SRX882 recorder and waveform analyzer as diagnostic tools only.
- Existing manufactured-command APIs now use the built-in generator automatically.
- Commands work immediately after boot or firmware update without any captured template in RAM or flash.
