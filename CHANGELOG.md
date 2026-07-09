# v1.3.21 – Raw OEM Edge Replay

- Adds `replay_last_outprize_raw_capture()` to replay the learned OEM capture edge deltas directly.
- Adds `replay_last_raw_capture()` alias for the most recent RF capture.
- Stores the learned capture initial GDO0 level and per-edge levels in addition to edge durations.
- Keeps known-frame replay, RF profiles, learned-frame diagnostics, compare diagnostics, and candidate diagnostics.
- Leaves reconstructed packet TX behavior unchanged.
