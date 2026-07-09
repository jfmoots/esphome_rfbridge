# v1.3.20 – Manual Known Frame Test Controls

- Adds `set_outprize_learned_frame(full35)` so a known frame can be forced into the learned-frame slot.
- Adds `replay_known_outprize_power_off()` for deterministic Power Off replay without depending on RX learning.
- Adds `compare_known_outprize_power_off()` for known-frame timing diagnostics.
- Keeps RF profile helpers from v1.3.19.
- Keeps full RF capture timeline, learned-frame, compare, and candidate diagnostics.
