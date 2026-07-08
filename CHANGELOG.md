# Changelog

## v1.1.4 – Outprize Remote ID Logger

- Updates firmware version to 1.1.4.
- Keeps the verified v1.0.0 Outprize decoder unchanged.
- Keeps the v1.1.3 configured remote ID TX API.
- Logs the decoded 11-bit Outprize remote prefix on full 35-bit receives.
- Logs the reconstructed full 35-bit frame as `full35` alongside `low24`.
- Marks clipped-but-valid command decodes as `remote=? full35=?` so they can still be used for Low24 verification without pretending the remote ID was learned.
- Clarifies that `remote_id` is the 11-bit Outprize RF prefix, not a 24-bit field.
