# Changelog

## v1.5.9

- Fixed the clean-build failure exposed after the production YAML cleanup.
- Added an intentional no-op `outprize_codec.cpp` compatibility translation unit.
- Ensures Git/ESPHome overwrites any stale bridge-coupled codec implementation left in an existing external-component worktree.
- Removes duplicate `capability_summary()` and obsolete `send_low24()` / `send_complete_state()` definitions from clean builds.
- Preserves all RF Bridge v1.5.8 boot/session, heartbeat, capability, Outprize RX, addressed TX, and STX882 behavior.
- No YAML changes from the cleaned production configuration.
