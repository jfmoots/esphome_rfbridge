# CC1101 Bring-Up

v0.3.0 keeps the native bit-banged SPI driver introduced during the v0.2.x bring-up work.

Expected healthy CC1101 values:

```text
PARTNUM = 0x00
VERSION = 0x14
```

The bridge configures the CC1101 for 433.92 MHz OOK asynchronous receive and maps GDO0 to async serial data output.

v0.3.0 adds a first receive pipeline that watches GDO0 for edges and logs RF packet candidates. It does not decode protocols yet.
