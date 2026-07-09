# v1.3.19 – RF Profile API Fix

This release fixes the v1.3.18 YAML/API mismatch by exposing the RF profile selector method that the test YAML uses.

Changes:
- Adds `send_outprize_power_off_profile(profile)` as a public C++ helper.
- Maps profile 1..6 to the RF characterization helpers already present in v1.3.18.
- Keeps the learned/proven Outprize Power Off payload unchanged.
- Keeps v1.3.17 full RF capture timeline diagnostics.
- Keeps learned-frame, learned-vs-TX compare, and 30–35 bit candidate diagnostics.
- Keeps v1.3.16 OEM-header transmitter behavior unchanged.

Goal:
Allow Home Assistant/ESPHome YAML to select RF characterization profiles without calling missing methods.
