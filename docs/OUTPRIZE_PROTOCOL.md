# Outprize RF Protocol

Reverse engineered from Outprize RV vent fan remote captures.

## Packet shape

Observed decoded payload:

```text
D9 EC 00 SS VV
```

For the user's remote, the learned remote ID / fixed prefix is:

```text
D9 EC 00
```

The low 24-bit value observed by the diagnostic decoder uses this base:

```text
0x600000
```

## Speed encoding

Speed uses a 4-bit Gray-code-like value shifted into the state field.

| Speed | Code nibble | Example Out, Rain Off, Vent None |
|---:|---:|---:|
| 0% | `0x0` | `0x600040` / power-state packet |
| 10% | `0x8` | `0x600440` |
| 20% | `0x4` | `0x600240` |
| 30% | `0xC` | `0x600640` |
| 40% | `0x2` | `0x600140` |
| 50% | `0xA` | `0x600540` |
| 60% | `0x6` | `0x600340` |
| 70% | `0xE` | `0x600740` |
| 80% | `0x1` | `0x6000C0` |
| 90% | `0x9` | `0x6004C0` |
| 100% | `0x5` | `0x6002C0` |

## State flags

| Flag | Meaning |
|---:|---|
| `0x10` | Rain sensor enabled |
| `0x20` | Intake / In direction |

## Vent command field

| Vent command | Modifier |
|---|---:|
| None | `0x00` |
| Close | `0x04` |
| Open | `0x08` |
| Stop | `0x0C` |

## Examples

| Meaning | Low24 |
|---|---:|
| Out 50%, rain off, no vent action | `0x600540` |
| In 50%, rain off, no vent action | `0x600560` |
| Out 50%, rain on, no vent action | `0x600550` |
| Out 50%, rain off, open vent | `0x600548` |
| Out 50%, rain off, close vent | `0x600544` |
| Out 50%, rain off, stop vent | `0x60054C` |
| Out 50%, rain on, open vent | `0x600558` expected |
| Out 90%, rain on | `0x6004D0` |

## Working encoding model

```cpp
low24 = 0x600000
      | (speed_code << 9)
      | (rain ? 0x10 : 0)
      | (in_direction ? 0x20 : 0)
      | vent_command;
```

The diagnostic decoder currently emits 35 bits using `PWM gap short=0 long=1`.
