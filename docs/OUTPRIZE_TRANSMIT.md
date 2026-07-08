# Outprize Transmit Notes

v1.1.0 adds the first experimental RF transmitter.

## Frame model

The transmitter sends a 35-bit frame:

```text
[prefix: 11 bits][Low24 payload: 24 bits]
```

The default prefix is `0x6CF`, derived from clean decoder captures.

## PWM shape

Observed Outprize RF uses gap-width PWM:

```text
reset low gap: ~7500 us
sync high:     ~4500 us
bit pulse:     low ~500 us
0 gap:         high ~500 us
1 gap:         high ~1500 us
```

Frames are repeated three times by default.

## Verified Low24 examples

```text
POWER OFF:         0x600000
FAN OFF / awake:   0x600040
60% OUT:           0x600340
60% OUT Rain ON:   0x600350
Vent Open:         +0x08
Vent Close:        +0x04
Vent Stop:         +0x0C
Direction IN:      +0x20
Rain ON:           +0x10
```

## Caution

This is the first TX build. Use with direct observation of the fan/vent and keep the OEM remote nearby.
