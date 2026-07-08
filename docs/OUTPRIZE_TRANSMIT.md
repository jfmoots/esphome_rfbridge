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
