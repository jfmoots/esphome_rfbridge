# esphome_rfbridge v1.3.24

## v1.3.24 – STX882/SRX882 Discrete ASK Backends

- Adds optional `stx882_data_pin`, `srx882_data_pin`, and `srx882_enable_pin` configuration.
- Adds direct STX882 raw learned-frame replay.
- Adds direct STX882 multi-frame sequence replay.
- Adds a deterministic known Power Off test through the STX882.
- Adds an explicit SRX882 raw capture window and SRX882-to-STX882 raw replay.
- Pauses the nearby CC1101 receiver while the STX882 transmits, then restores CC1101 RX.
- Adds Home Assistant-visible SRX882 capture status helpers.
- Makes command learning deliberate/gated: normal RF traffic outside an active learn window no longer overwrites the learned command.
- Starting a new learn window clears the previous learned command so status cannot falsely show stale data.
