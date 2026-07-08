# Changelog

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
