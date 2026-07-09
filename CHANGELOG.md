# v1.3.17 – Full RF Capture Diagnostics

- Keeps the v1.3.16 OEM-header transmitter unchanged.
- Keeps learned-frame capture diagnostics.
- Keeps learned-vs-TX edge comparison diagnostics.
- Keeps 30–35 bit candidate alignment diagnostics.
- Adds full RSSI-gated capture timeline logging with cumulative edge timing and GDO0 level markers.
- Marks decoder start/stop edges on the timeline when a learned Outprize frame is present.
- Logs first-edge delay from RSSI trigger and pre-data edge deltas so header/preamble alignment can be inspected directly.
- Increases the diagnostic capture edge buffer to reduce overrun risk during noisy/long captures.
- Leaves payload, transmit timing, bit order, CC1101 TX/RX configuration, frequency helpers, and RX restore behavior unchanged.
