# Codec Contract

Every codec implements `RFBridgeCodec` and reports:

- stable codec ID
- codec API version
- preferred receive backend
- preferred transmit backend
- optional diagnostic receive backend
- capability summary

Current codec:

```text
outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882
```

Future codecs are added under `esphome/components/rfbridge/codecs/<codec_id>/` and compiled into the firmware. Home Assistant integrations do not upload executable codec code to the ESP. They discover and require a compatible codec through bridge capability reporting.
