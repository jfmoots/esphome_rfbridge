# ESPHome RF Bridge v1.3.13

ESPHome external component for an ESP32 + CC1101 Outprize RF bridge.

v1.3.13 is a diagnostics-only release. It stores the raw edge timings from the OEM capture that produced `OUTPRIZE_LEARNED` and adds a helper to compare that learned OEM pulse train against the ESP transmitter's generated pulse train.

New helper:

```cpp
id(rf_bridge).compare_last_outprize_learned();
```

Typical test flow:

1. Flash v1.3.13 with diagnostic logging enabled.
2. Press the OEM remote Power Off button and wait for `OUTPRIZE_LEARNED`.
3. Press a Home Assistant button calling `compare_last_outprize_learned()`.
4. Review the `OUTPRIZE_COMPARE` log output.

The transmitter, receiver, CC1101 settings, frequency-trim helpers, and waveform timing from v1.3.12.1 are otherwise unchanged.
