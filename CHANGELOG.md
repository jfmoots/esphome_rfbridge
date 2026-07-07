# Changelog

## v0.4.2 - Known-Good Register Readback Profile

- Updates CC1101 receive configuration to match values observed in the previous working RF Gateway/sniffer readback.
- Sets IOCFG2=0x29, IOCFG0=0x0D, PKTCTRL0=0x30, and PKTLEN=0xFF.
- Updates diagnostics expected values for those registers.
- Keeps v0.4.x register dump and RX activity diagnostics.


## v0.4.1 – Match Known-Good Sniffer Profile

- Updates the CC1101 modem configuration to better match the working standalone diagnostic sniffer.
- Changes MDMCFG4/MDMCFG3 to the 2.0 kbps / ~58 kHz RX bandwidth profile used by the original RadioLib capture firmware.
- Keeps IOCFG0=0x0D, IOCFG2=0x2E, and PKTCTRL0=0x32 for asynchronous serial data on GDO0.
- Updates register diagnostics expected values to match the new profile.
- Keeps version/build metadata and receive diagnostics from v0.4.0.

No remote button press is required for basic bring-up; this release prepares the receiver for the next real Outprize remote test.
