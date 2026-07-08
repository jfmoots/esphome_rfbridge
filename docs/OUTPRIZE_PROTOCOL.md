# Outprize RF Protocol Notes

This document is the source-of-truth protocol model for the MooterHome Outprize vent fan remote.

## Low24 packet family

Verified packets decode into the `0x60xxxx` family.

## Speed table, OUT, rain off, vent idle

| Speed | Low24 |
|---:|---:|
| OFF / wake / fan-off awake | `0x600040` |
| 10% | `0x600440` |
| 20% | `0x600240` |
| 30% | `0x600640` |
| 40% | `0x600140` |
| 50% | `0x600540` |
| 60% | `0x600340` |
| 70% | `0x600740` |
| 80% | `0x6000C0` |
| 90% | `0x6004C0` |
| 100% | `0x6002C0` |

## Modifiers

| Field | Modifier |
|---|---:|
| Direction IN | `+0x20` |
| Rain enabled | `+0x10` |
| Vent close | `+0x04` |
| Vent open | `+0x08` |
| Vent stop | `+0x0C` |

## Stateful button behavior

| Action | Low24 | Observed behavior |
|---|---:|---|
| POWER OFF | `0x600000` | Remote/display off, vent closes, fan stops |
| POWER ON | `0x600040` | Remote/display wakes; fan and vent do not move |
| FAN when fan off | remembered speed packet | Starts fan at remembered speed/direction/rain state; vent does not open |
| FAN when fan on | `0x600040` | Stops fan, leaves controller awake |

## Implementation notes

The remote is stateful. Repeated presses of a button are not necessarily repeated commands. For example, pressing `+` walks the speed state through the Gray-coded table, and pressing FAN toggles fan state. Decoder verification should compare decoded values to expected state transitions, not repeated identical button presses.
