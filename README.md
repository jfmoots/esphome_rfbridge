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
          id(rf_bridge).send_outprize_low24(0x600140);

  - platform: template
    name: "Outprize TX Test Fan Off Awake"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600040);

  - platform: template
    name: "Outprize TX Test Power Off"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600000);
```
