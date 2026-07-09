# ESPHome RF Bridge

## v1.3.6 - Full CC1101 TX Profile

This build makes the CC1101 async transmit model explicit. During receive, GDO0 is the CC1101 async data output and the ESP reads it. During transmit, the ESP temporarily switches the same GPIO to output and drives GDO0 as the CC1101 async TX data/envelope input. The 500 ms carrier test forces GDO0 HIGH while the CC1101 is in TX, then restores GDO0 to input and returns the radio to RX.


Build metadata compile-fix release for OEM waveform replay testing.

# ESPHome RF Bridge

## v1.1.4 – Outprize Remote ID Logger

This release keeps the verified Outprize decoder and experimental TX test interface, but adds the missing receive-side visibility needed before serious TX testing: the decoded 11-bit Outprize remote prefix and reconstructed 35-bit frame.

Normal receive logs now look like:

```text
OUTPRIZE remote=0x6CF full35=0x6CF600340 low24=0x600340 confidence=excellent ...
```

If a capture is clipped but the Low24 command can still be decoded, it logs:

```text
OUTPRIZE remote=? full35=? low24=0x600340 ...
```

Use several full 35-bit captures to confirm the remote prefix is stable, then set it explicitly in YAML before TX testing.

```yaml
rfbridge:
  id: rf_bridge
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
  diagnostic_logging: false
  remote_id: 0x6CF
```

Temporary TX smoke-test buttons:

```yaml
button:
  - platform: template
    name: "Outprize TX Test 40 Percent OUT"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600140, 8);

  - platform: template
    name: "Outprize TX Test Fan Off Awake"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600040, 8);

  - platform: template
    name: "Outprize TX Test Power Off"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600000, 8);
```


## v1.3.0 OEM waveform replay test

This release adds a raw replay smoke test for the transmit path. Use it before relying on generated Outprize packets.

Test flow:

1. Flash v1.3.0.
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


## v1.3.0 TX Hardware Verification

This release adds a protocol-neutral OOK hardware test burst. Use the `RF TX Hardware Test Burst` template button to confirm that the CC1101 can emit a visible 433.92 MHz OOK burst before debugging Outprize packet synthesis.

Example lambda:

```yaml
- lambda: |-
    id(rf_bridge).send_ook_test_burst(500, 240, 8);
```

The test emits an alternating OOK pulse train using 500 µs pulse spacing, 120 transitions, and 3 repeats, then restores the CC1101 to RX mode.


## v1.3.3 Non-blocking carrier test

This release keeps the verified Outprize decoder intact and improves the transmit test path:

- Uses PA table `0xC0` for maximum test output.
- Raises test repeats to 8 in the example buttons.
- Logs estimated TX duration.
- Logs CC1101 `MARCSTATE` before TX configuration, after TX configuration, after `STX`, before idle restore, and after idle restore.
- Restores RX mode after each TX test.

Use this release to compare SDR-visible output from the bridge against the OEM remote before continuing packet-shape tuning.


## v1.3.3 Non-blocking Carrier Test

Use `id(rf_bridge).send_ook_carrier_test(500);` from a template button to hold the CC1101 in async OOK TX for a short non-blocking carrier test. The carrier is stopped from `loop()` and RX is restored after the timeout, avoiding API disconnects/watchdog stalls from long blocking button handlers.
