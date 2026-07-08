#pragma once

#include <cstdint>

namespace esphome {
namespace rfbridge {
namespace cc1101 {

// CC1101 configuration registers
static constexpr uint8_t IOCFG2 = 0x00;
static constexpr uint8_t IOCFG1 = 0x01;
static constexpr uint8_t IOCFG0 = 0x02;
static constexpr uint8_t FIFOTHR = 0x03;
static constexpr uint8_t SYNC1 = 0x04;
static constexpr uint8_t SYNC0 = 0x05;
static constexpr uint8_t PKTLEN = 0x06;
static constexpr uint8_t PKTCTRL1 = 0x07;
static constexpr uint8_t PKTCTRL0 = 0x08;
static constexpr uint8_t ADDR = 0x09;
static constexpr uint8_t CHANNR = 0x0A;
static constexpr uint8_t FSCTRL1 = 0x0B;
static constexpr uint8_t FSCTRL0 = 0x0C;
static constexpr uint8_t FREQ2 = 0x0D;
static constexpr uint8_t FREQ1 = 0x0E;
static constexpr uint8_t FREQ0 = 0x0F;
static constexpr uint8_t MDMCFG4 = 0x10;
static constexpr uint8_t MDMCFG3 = 0x11;
static constexpr uint8_t MDMCFG2 = 0x12;
static constexpr uint8_t MDMCFG1 = 0x13;
static constexpr uint8_t MDMCFG0 = 0x14;
static constexpr uint8_t DEVIATN = 0x15;
static constexpr uint8_t MCSM2 = 0x16;
static constexpr uint8_t MCSM1 = 0x17;
static constexpr uint8_t MCSM0 = 0x18;
static constexpr uint8_t FOCCFG = 0x19;
static constexpr uint8_t BSCFG = 0x1A;
static constexpr uint8_t AGCCTRL2 = 0x1B;
static constexpr uint8_t AGCCTRL1 = 0x1C;
static constexpr uint8_t AGCCTRL0 = 0x1D;
static constexpr uint8_t WOREVT1 = 0x1E;
static constexpr uint8_t WOREVT0 = 0x1F;
static constexpr uint8_t WORCTRL = 0x20;
static constexpr uint8_t FREND1 = 0x21;
static constexpr uint8_t FREND0 = 0x22;
static constexpr uint8_t FSCAL3 = 0x23;
static constexpr uint8_t FSCAL2 = 0x24;
static constexpr uint8_t FSCAL1 = 0x25;
static constexpr uint8_t FSCAL0 = 0x26;
static constexpr uint8_t RCCTRL1 = 0x27;
static constexpr uint8_t RCCTRL0 = 0x28;
static constexpr uint8_t FSTEST = 0x29;
static constexpr uint8_t PTEST = 0x2A;
static constexpr uint8_t AGCTEST = 0x2B;
static constexpr uint8_t TEST2 = 0x2C;
static constexpr uint8_t TEST1 = 0x2D;
static constexpr uint8_t TEST0 = 0x2E;

// Strobes
static constexpr uint8_t SRES = 0x30;
static constexpr uint8_t SFSTXON = 0x31;
static constexpr uint8_t SXOFF = 0x32;
static constexpr uint8_t SCAL = 0x33;
static constexpr uint8_t SRX = 0x34;
static constexpr uint8_t STX = 0x35;
static constexpr uint8_t SIDLE = 0x36;
static constexpr uint8_t SAFC = 0x37;
static constexpr uint8_t SWOR = 0x38;
static constexpr uint8_t SPWD = 0x39;
static constexpr uint8_t SFRX = 0x3A;
static constexpr uint8_t SFTX = 0x3B;
static constexpr uint8_t SWORRST = 0x3C;
static constexpr uint8_t SNOP = 0x3D;

// Multi-byte / special registers
static constexpr uint8_t PATABLE = 0x3E;
static constexpr uint8_t TXFIFO = 0x3F;
static constexpr uint8_t RXFIFO = 0x3F;

// Status registers
static constexpr uint8_t PARTNUM = 0x30;
static constexpr uint8_t VERSION = 0x31;
static constexpr uint8_t RSSI = 0x34;
static constexpr uint8_t MARCSTATE = 0x35;
static constexpr uint8_t PKTSTATUS = 0x38;
static constexpr uint8_t FREQEST = 0x32;
static constexpr uint8_t TXBYTES = 0x3A;
static constexpr uint8_t RXBYTES = 0x3B;

// SPI command bits
static constexpr uint8_t READ_SINGLE = 0x80;
static constexpr uint8_t READ_BURST = 0xC0;
static constexpr uint8_t WRITE_BURST = 0x40;

// Values observed from the known-good ESPHome RF Gateway/sniffer register readback.
static constexpr uint8_t GDO_SERIAL_DATA = 0x0D;   // async serial data input/output for async serial mode
static constexpr uint8_t GDO_IOCFG2_KNOWN_GOOD = 0x29;
static constexpr uint8_t PKTCTRL0_KNOWN_GOOD = 0x30;  // known-good working sniffer value
static constexpr uint8_t PKTCTRL0_ASYNC_INFINITE = 0x32;  // async serial mode + infinite length baseline for TX diagnostics

}  // namespace cc1101
}  // namespace rfbridge
}  // namespace esphome
