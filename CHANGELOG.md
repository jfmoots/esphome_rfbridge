# v1.3.7 – Carrier-Off Inter-Frame Fix

This release continues Outprize transmit waveform reconstruction after v1.3.6 produced a visible, timing-close ESP waveform but no fan response.

Changes:
- Keeps the working CC1101 TX profile, PATABLE, calibration, and RX restore path unchanged.
- Treats ESP LOW on GDO0 as carrier-on and ESP HIGH as carrier-off for the async OOK envelope.
- Removes the unwanted carrier-on reset/inter-frame behavior that created repeated ~6.1 ms leader pulses in rtl_433 captures.
- Leaves the proven 488 us / 976 us symbol timing intact.
- Uses carrier-off spacing between repeated frames.
- Adds a single short leading sync/delimiter pulse instead of a long per-frame carrier-on leader.

Goal:
Move the ESP Power Off capture closer to the OEM remote by eliminating the extra ~6 ms pulses while preserving the now-correct core bit timing.

# Changelog

## v1.3.6 – Outprize Waveform Match Pass

- Keeps the v1.3.5 CC1101 TX profile and calibration path unchanged.
- Adjusts Outprize TX waveform timing to better match paired rtl_433 -A captures from the OEM remote.
- Changes base pulse timing to 488 us.
- Changes zero and one gaps to 488 us and 1464 us.
- Changes reset/sync/inter-frame timing toward the observed OEM repeat spacing.
- Adds clearer TX waveform diagnostics showing bit count, ones/zeros, timing constants, repeat count, and estimated burst duration.
- RX decoder path remains unchanged.

# Changelog

## v1.3.6 - Full CC1101 TX Profile

- Updates firmware version to 1.3.6.
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
