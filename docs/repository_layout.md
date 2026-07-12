# Repository Layout

```text
README.md                  Project overview and quick start
CHANGELOG.md               Release history

docs/
  architecture.md          Responsibility boundaries and data flow
  hardware.md              Reference build and RF installation
  installation.md          ESPHome and Home Assistant setup
  api.md                   Current events and API actions
  codec_development.md     Adding protocol support
  reverse_engineering.md   Canonical Outprize findings and project history
  repository_layout.md     This file

esphome/
  components/
    rfbridge/
      __init__.py          ESPHome schema and code generation
      rfbridge.h           Bridge component interface/state
      rfbridge.cpp         Radio, capture, routing, and transport implementation
      codec.h              Generic codec abstraction
      outprize_codec.h     Outprize protocol model/codec
      outprize_codec.cpp   Clean-build compatibility translation unit
      outprize_codec_impl.h Compatibility shim for stale ESPHome worktrees
      cc1101_regs.h        CC1101 register definitions
      version.h            Firmware metadata

  examples/
    rfbridge_production.yaml
```

## Current packaging constraint

ESPHome external-component source copying and cached worktrees can preserve stale translation units across upgrades. The intentionally minimal `outprize_codec.cpp` and compatibility shim ensure clean and incremental builds converge on the same pure-codec implementation.

Do not remove these files solely because they appear empty without first validating both a clean ESPHome build and an upgrade from older component layouts.

## Future direction

As additional codecs and backends are added, the internal source layout may evolve toward explicit subdirectories. Any change must account for ESPHome external-component source discovery and must be clean-build tested before release.
