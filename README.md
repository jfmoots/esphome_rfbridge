# esphome_rfbridge v1.3.9

Outprize RF Bridge ESPHome external component.

v1.3.9 keeps the proven v1.3.8 RF timing/envelope and adds transmit-content diagnostics plus alternate packet-content test modes. The goal is to stop chasing pulse timing and verify whether the remaining fan-control failure is caused by bit order, inversion, or full-frame construction.

## Highlights

- Receiver/decoder path unchanged.
- CC1101 TX profile, calibration, PATABLE, and RX restore path unchanged.
- v1.3.8 waveform timing retained.
- Logs the exact 35-bit frame stream before transmit.
- Adds callable test methods for MSB, LSB, inverted MSB, inverted LSB, and raw full35 transmission.

## Example lambda test buttons

```yaml
button:
  - platform: template
    name: "Outprize TX Power Off MSB Normal"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x600000);

  - platform: template
    name: "Outprize TX Power Off LSB Normal"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24_lsb(0x600000);

  - platform: template
    name: "Outprize TX Power Off MSB Inverted"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24_inverted(0x600000);

  - platform: template
    name: "Outprize TX Power Off LSB Inverted"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24_lsb_inverted(0x600000);

  - platform: template
    name: "Outprize TX Raw Full35 Power Off"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_raw_full35(0x6CF600000ULL);
```
