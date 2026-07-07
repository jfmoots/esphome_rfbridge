# Diagnostic Firmware

This is the standalone PlatformIO/Arduino firmware used during reverse engineering.

It intentionally remains separate from the ESPHome component because it is a diagnostic tool, not production firmware.

It uses RadioLib and direct SPI/register access to sniff Outprize RF packets and print decoded PWM gap packets.
