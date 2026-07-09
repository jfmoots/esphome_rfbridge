# v1.3.18 – TX RF Mode Characterization

This release keeps the learned/proven Outprize payload and timing unchanged, and adds radio-layer transmit profile experiments.

Changes:
- Keeps v1.3.17 full RF capture timeline diagnostics.
- Keeps learned-frame capture diagnostics.
- Keeps learned-vs-TX edge comparison diagnostics.
- Keeps 30–35 bit candidate alignment diagnostics.
- Keeps the OEM Power Off payload unchanged.
- Adds Power Off RF profile helpers that vary only TX radio-layer behavior:
  - default async OOK baseline
  - inverted async OOK electrical polarity
  - lower PATABLE PA levels
  - alternate FREND0
  - alternate MDMCFG2
- Logs the selected PA, FREND0, MDMCFG2, and inverted-OOK state during TX setup.

Goal:
Determine whether the fan is rejecting the ESP transmission because of RF/electrical transmitter behavior rather than payload, bit order, or timing.
