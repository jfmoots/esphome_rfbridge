# v1.3.16 – OEM Header Timing Match

- Keeps learned-frame capture diagnostics.
- Keeps learned-vs-TX edge comparison diagnostics.
- Keeps 30–35 bit candidate alignment diagnostics.
- Replaces the previous synthetic TX startup header with OEM-style timing.
- Uses a ~4596 us carrier-on lead-in followed by a ~4513 us carrier-off header gap before the PWM payload.
- Keeps payload, bit order, CC1101 TX/RX configuration, frequency helpers, and RX restore behavior unchanged.
