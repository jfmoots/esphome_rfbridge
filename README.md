# RF Bridge v1.5.9

Clean-build compatibility release for the production RF Bridge firmware.

This release keeps the v1.5.8 runtime behavior unchanged while ensuring that
ESPHome clean builds cannot compile an obsolete bridge-coupled
`outprize_codec.cpp` left in the external-component source tree.

Use the same cleaned production YAML supplied for v1.5.8.
