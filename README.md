# ESPHome RF Bridge

## v1.1.2 – Lambda TX Test Interface

This release keeps the verified Outprize decoder baseline and adds a practical transmit test interface that ESPHome can compile today.

The v1.1.1 custom actions were the right idea, but ESPHome did not recognize them from YAML in the current external-component layout. v1.1.2 uses direct ESPHome lambdas against the component ID instead:

```yaml
- lambda: |-
    id(rf_bridge).send_outprize_low24(0x6CF, 0x600140, 3);
```

That lets us test the transmitter without waiting on the custom action registration path.

## Test YAML

Use `esphome/examples/rfbridge_outprize_transmit_test.yaml` as the reference.

Minimum important pieces:

```yaml
rfbridge:
  id: rf_bridge
  cs_pin: GPIO5
  sck_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19
  gdo0_pin: GPIO4
  diagnostic_logging: false

button:
  - platform: template
    name: "Outprize TX Test 40 Percent OUT"
    on_press:
      - lambda: |-
          id(rf_bridge).send_outprize_low24(0x6CF, 0x600140, 3);
```

## Optional raw transmit service

The example also exposes a generic Home Assistant-callable API service:

```yaml
api:
  services:
    - service: outprize_send_low24
      variables:
        low24: int
        remote_id: int
        repeats: int
      then:
        - lambda: |-
            id(rf_bridge).send_outprize_low24((uint32_t) remote_id, (uint32_t) low24, (uint8_t) repeats);
```

This is closer to the eventual architecture: the Home Assistant Outprize integration computes the Low24 packet and the bridge transmits it.

## Verified Outprize Low24 examples

- `0x600000` – POWER OFF: display off, fan stops, vent closes
- `0x600040` – awake idle / fan off, vent unchanged
- `0x600140` – 40% OUT
- `0x600340` – 60% OUT
- `0x600170` – 40% IN + Rain
- `0x600178` – 40% IN + Rain + Vent OPEN
- `0x60017C` – 40% IN + Rain + Vent STOP
- `0x600154` – 40% OUT + Rain + Vent CLOSE

## Current goal

This release is not yet the final Outprize control integration. It is for one thing: proving that the ESP32 + CC1101 can transmit a verified Outprize packet and make the fan respond.
