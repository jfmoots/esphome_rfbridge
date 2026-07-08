# Changelog

## v1.3.5 - Full CC1101 TX Profile

- Updates firmware version to 1.3.5.
- Replaces the minimal TX setup with an explicit full CC1101 async OOK TX register profile.
- Changes the TX diagnostic PA table value from 0x84 to 0xC0.
- Uses async serial + infinite packet baseline (`PKTCTRL0 = 0x32`) for TX diagnostics.
- Adds explicit `SCAL` calibration before `STX` on Outprize TX, carrier test, hardware burst test, and replay.
- Adds TX status dumps around TX configuration, calibration, and STX (`MARCSTATE`, `PKTSTATUS`, `TXBYTES`, `FREQEST`, RSSI).
- Keeps the verified RX decoder and RX restore path intact.

## v1.3.4 - Drive GDO0 During TX

- Updates firmware version to 1.3.4.
- Makes the CC1101 async TX assumption explicit: ESP drives GDO0 as the TX data/envelope input.
- Adds log lines when GDO0 is switched from RX input to TX output and back.
- Forces GDO0 HIGH during the carrier test and LOW before restoring RX.
- Keeps the non-blocking 500 ms carrier test from v1.3.3.
- Keeps the verified Outprize decoder intact.

## v1.3.3 - Non-blocking Carrier Test

- Updates firmware version to 1.3.3.
- Changes the carrier test path from blocking `delay()` to a loop-driven state machine.
- Defaults the example carrier test to 500 ms instead of 5 seconds.
- Keeps RX restore after carrier timeout.
- Uses a named CC1101 TX PA test value instead of hard-coded PA bytes.
- Keeps the verified Outprize decoder and existing TX helpers intact.

# Changelog

## v1.3.2 - Long Carrier TX Test

- Adds `send_ook_carrier_test(duration_ms)` for a visually obvious SDR test.
- Adds example `RF TX 5 Second Carrier Test` button.
- Keeps v1.3.1 state logging and verified Outprize decoder.
- Restores RX mode after carrier test.
