# v1.3.15 – Power Off Alignment Diagnostics

This release focuses on diagnosing why OEM Power Off frames do not learn cleanly while Fan Awake frames do.

Changes:
- Keeps transmitter behavior unchanged.
- Keeps CC1101 TX/RX configuration unchanged.
- Adds top-candidate alignment logging for 30–35 bit Outprize-like decodes.
- Logs candidate start/stop index, bit count, score, prefix, low24, full frame, and bitstream.
- Flags likely clipped/misaligned Power Off candidates.
- Leaves YAML API unchanged.
