# ESPHome RF Bridge v1.3.10

ESPHome external component for the Outprize RF bridge project using ESP32 + CC1101.

## v1.3.10 focus

v1.3.10 implements the Power Off frame test helper methods that are intended to be called from ESPHome template buttons during protocol reconstruction.

The RF timing and CC1101 TX setup are unchanged from the recent waveform-matching builds.

## Test helpers available from lambda

```cpp
id(rf_bridge).send_outprize_low24(0x600000);
id(rf_bridge).send_outprize_power_off();
id(rf_bridge).send_outprize_power_off_lsb();
id(rf_bridge).send_outprize_power_off_inv();
id(rf_bridge).send_outprize_power_off_inv_lsb();
id(rf_bridge).send_outprize_raw_oem_power_off();
id(rf_bridge).send_outprize_raw_full35(0x6CF600000ULL);
```

## Hardware

Known-good wiring for this project:

- CS -> GPIO5
- SCK -> GPIO18
- MOSI -> GPIO23
- MISO -> GPIO19
- GDO0 -> GPIO4
- 3.3V
- GND

GDO2 is not used.
