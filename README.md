# ESPHome RF Bridge v1.3.17

This release adds full RF capture timeline diagnostics while keeping the v1.3.16 transmitter behavior unchanged.

The goal is to understand what the ESP actually captures from the OEM remote before and around the decoded Outprize frame start. The new diagnostics print the RSSI-triggered capture as cumulative edge timings and mark the learned frame boundaries when available.

No YAML changes are required from v1.3.16.
