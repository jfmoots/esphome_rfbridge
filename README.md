# ESPHome RF Bridge

Reusable ESPHome RF bridge for CC1101-based 433 MHz experiments.

## v0.5.0

This release changes receive bring-up to match the original working Outprize diagnostic sniffer shape: RSSI-gated fixed-window capture. The bridge ignores idle GDO0 noise until RSSI crosses -80 dBm, then captures a 140 ms GDO0 timing window for later decoding.
