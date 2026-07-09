# esphome_rfbridge v1.3.23

## v1.3.23 – Multi-Frame Learn Mode & HA Status Indicators

- Adds an explicit multi-frame RF sequence learning mode.
- Adds raw replay for the captured sequence.
- Adds public status helpers for Home Assistant template text/binary sensors.
- Keeps learned decoded-frame replay, learned raw-capture replay, RF capture diagnostics, and Soft ASK profiles.
- Improves troubleshooting by surfacing whether a command is actually learned and what it decoded to.
