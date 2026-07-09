# Outprize Transmit Notes

The Outprize RF frame is treated as:

```text
35 bits total = 11-bit remote prefix + 24-bit command Low24
```

The verified command field is the Low24 value, for example:

```text
0x600340 = 60% OUT
0x600040 = fan off / awake idle
0x600000 = power off / close vent / fan stop
```

v1.1.4 logs the learned receive prefix as `remote=0x...` and the reconstructed frame as `full35=0x...`.

Set the learned prefix in YAML:

```yaml
rfbridge:
  remote_id: 0x6CF
```

The low-level TX test API then sends using the configured prefix:

```cpp
id(rf_bridge).send_outprize_low24(0x600340);
```

The hard-coded buttons are only for RF smoke testing. The long-term Home Assistant integration should compute/choose the Low24 command and ask the bridge to transmit it.


## v1.2.0 OEM waveform replay test

This release adds a raw replay smoke test for the transmit path. Use it before relying on generated Outprize packets.

Test flow:

1. Flash v1.2.0.
2. Press one OEM Outprize remote button and confirm the bridge logs an `OUTPRIZE` decode.
3. Press the ESPHome button `RF Replay Last Capture`.
4. Watch whether the fan repeats the same behavior.

The replay method uses the last captured pulse train rather than synthesizing bits from `remote_id + low24`. This helps separate CC1101 transmit/modulation issues from packet-generation issues.

Example button:

```yaml
button:
  - platform: template
    name: "RF Replay Last Capture"
    on_press:
      - lambda: |-
          id(rf_bridge).replay_last_capture(1);
```


## v1.3.6 waveform matching notes

Paired rtl_433 -A captures showed that v1.3.5 was radiating but its burst was longer than the OEM remote (about 549 ms vs about 394 ms for Power Off). The v1.3.6 pass keeps the working CC1101 transmit configuration and adjusts only the software-generated Outprize waveform timing. The target baseline is approximately 488 us short cells, 976 us full cells, and about 3040 us repeat spacing.
