#include "rfbridge.h"
#include "cc1101_regs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rfbridge {

static const char *const TAG = "rfbridge";

void RFBridgeComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RF Bridge...");

  this->spi_setup();

  if (this->gdo0_pin_ != nullptr) {
    this->gdo0_pin_->setup();
  }
  if (this->gdo2_pin_ != nullptr) {
    this->gdo2_pin_->setup();
  }

  if (!this->cc1101_begin_()) {
    this->mark_failed();
    return;
  }

  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();

  ESP_LOGCONFIG(TAG, "RF Bridge setup complete");
}

void RFBridgeComponent::loop() {
  // Receive/decode path will be added after the CC1101 foundation is validated.
}

void RFBridgeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RF Bridge");
  LOG_PIN("  GDO0 Pin: ", this->gdo0_pin_);
  LOG_PIN("  GDO2 Pin: ", this->gdo2_pin_);
}

bool RFBridgeComponent::cc1101_begin_() {
  ESP_LOGCONFIG(TAG, "Initializing CC1101 without RadioLib...");

  this->cc1101_reset_();

  const uint8_t partnum = this->cc1101_read_status_(cc1101::PARTNUM);
  const uint8_t version = this->cc1101_read_status_(cc1101::VERSION);

  ESP_LOGCONFIG(TAG, "CC1101 PARTNUM=0x%02X VERSION=0x%02X", partnum, version);

  // Common CC1101 modules usually report VERSION 0x14 or 0x04.
  // Don't hard-fail on unusual clones yet; just warn loudly.
  if (version == 0x00 || version == 0xFF) {
    ESP_LOGE(TAG, "CC1101 did not respond correctly. Check power, CS, and SPI wiring.");
    return false;
  }

  return true;
}

void RFBridgeComponent::cc1101_reset_() {
  // TI reset sequence: CS high -> low -> high, then SRES strobe.
  this->disable();
  delayMicroseconds(5);
  this->enable();
  delayMicroseconds(10);
  this->disable();
  delayMicroseconds(40);
  this->cc1101_strobe_(cc1101::SRES);
  delay(1);
}

void RFBridgeComponent::cc1101_configure_ook_async_rx_() {
  ESP_LOGCONFIG(TAG, "Configuring CC1101 for 433.92 MHz OOK async RX...");

  this->cc1101_enter_idle_();

  // Frequency: 433.92 MHz with a 26 MHz crystal.
  // FREQ = f_carrier * 2^16 / f_xtal = 0x10B071.
  this->cc1101_write_reg_(cc1101::FREQ2, 0x10);
  this->cc1101_write_reg_(cc1101::FREQ1, 0xB0);
  this->cc1101_write_reg_(cc1101::FREQ0, 0x71);

  // Async serial mode on GDO0. These three values were proven in the diagnostic firmware.
  this->cc1101_write_reg_(cc1101::IOCFG0, cc1101::GDO_SERIAL_DATA);
  this->cc1101_write_reg_(cc1101::IOCFG2, cc1101::GDO_HIGH_Z);
  this->cc1101_write_reg_(cc1101::PKTCTRL0, cc1101::PKT_ASYNC_SERIAL);

  // Minimal OOK/ASK-ish baseline. These are deliberately conservative first-pass settings.
  // We will tune them against the known-good diagnostic firmware logs if needed.
  this->cc1101_write_reg_(cc1101::FSCTRL1, 0x06);
  this->cc1101_write_reg_(cc1101::MDMCFG4, 0xF5);  // ~58 kHz channel BW, low data rate exponent
  this->cc1101_write_reg_(cc1101::MDMCFG3, 0x83);  // ~2 kbaud data rate mantissa
  this->cc1101_write_reg_(cc1101::MDMCFG2, 0x30);  // ASK/OOK, no sync filtering
  this->cc1101_write_reg_(cc1101::MDMCFG1, 0x22);
  this->cc1101_write_reg_(cc1101::MDMCFG0, 0xF8);
  this->cc1101_write_reg_(cc1101::MCSM0, 0x18);
  this->cc1101_write_reg_(cc1101::FOCCFG, 0x16);
  this->cc1101_write_reg_(cc1101::AGCCTRL2, 0x04);
  this->cc1101_write_reg_(cc1101::AGCCTRL1, 0x00);
  this->cc1101_write_reg_(cc1101::AGCCTRL0, 0x91);
  this->cc1101_write_reg_(cc1101::FREND1, 0x56);
  this->cc1101_write_reg_(cc1101::FREND0, 0x11);
  this->cc1101_write_reg_(cc1101::FSCAL3, 0xE9);
  this->cc1101_write_reg_(cc1101::FSCAL2, 0x2A);
  this->cc1101_write_reg_(cc1101::FSCAL1, 0x00);
  this->cc1101_write_reg_(cc1101::FSCAL0, 0x1F);
  this->cc1101_write_reg_(cc1101::TEST2, 0x81);
  this->cc1101_write_reg_(cc1101::TEST1, 0x35);
  this->cc1101_write_reg_(cc1101::TEST0, 0x09);

  this->cc1101_write_patable_(0xC0);

  ESP_LOGCONFIG(TAG, "  IOCFG0   = 0x%02X", this->cc1101_read_reg_(cc1101::IOCFG0));
  ESP_LOGCONFIG(TAG, "  IOCFG2   = 0x%02X", this->cc1101_read_reg_(cc1101::IOCFG2));
  ESP_LOGCONFIG(TAG, "  PKTCTRL0 = 0x%02X", this->cc1101_read_reg_(cc1101::PKTCTRL0));
}

void RFBridgeComponent::cc1101_enter_rx_() {
  this->cc1101_strobe_(cc1101::SFRX);
  this->cc1101_strobe_(cc1101::SRX);
}

void RFBridgeComponent::cc1101_enter_idle_() { this->cc1101_strobe_(cc1101::SIDLE); }

void RFBridgeComponent::cc1101_write_reg_(uint8_t addr, uint8_t value) {
  this->enable();
  delayMicroseconds(5);
  this->write_byte(addr);
  this->write_byte(value);
  delayMicroseconds(5);
  this->disable();
}

uint8_t RFBridgeComponent::cc1101_read_reg_(uint8_t addr) {
  this->enable();
  delayMicroseconds(5);
  this->write_byte(addr | cc1101::READ_SINGLE);
  const uint8_t value = this->read_byte();
  delayMicroseconds(5);
  this->disable();
  return value;
}

uint8_t RFBridgeComponent::cc1101_read_status_(uint8_t addr) {
  this->enable();
  delayMicroseconds(5);
  this->write_byte(addr | cc1101::READ_BURST);
  const uint8_t value = this->read_byte();
  delayMicroseconds(5);
  this->disable();
  return value;
}

uint8_t RFBridgeComponent::cc1101_strobe_(uint8_t strobe) {
  this->enable();
  delayMicroseconds(5);
  const uint8_t status = this->transfer_byte(strobe);
  delayMicroseconds(5);
  this->disable();
  return status;
}

void RFBridgeComponent::cc1101_write_patable_(uint8_t value) {
  this->enable();
  delayMicroseconds(5);
  this->write_byte(cc1101::PATABLE);
  this->write_byte(value);
  delayMicroseconds(5);
  this->disable();
}

uint8_t RFBridgeComponent::outprize_speed_code_(uint8_t speed_percent) const {
  switch (speed_percent) {
    case 0: return 0x0;
    case 10: return 0x8;
    case 20: return 0x4;
    case 30: return 0xC;
    case 40: return 0x2;
    case 50: return 0xA;
    case 60: return 0x6;
    case 70: return 0xE;
    case 80: return 0x1;
    case 90: return 0x9;
    case 100: return 0x5;
    default:
      return this->outprize_speed_code_(static_cast<uint8_t>(((speed_percent + 5) / 10) * 10));
  }
}

uint32_t RFBridgeComponent::encode_outprize_low24(uint8_t speed_percent, OutprizeDirection direction,
                                                  bool rain_enabled, OutprizeVentCommand vent_command) const {
  uint32_t low24 = 0x600000;
  low24 |= static_cast<uint32_t>(this->outprize_speed_code_(speed_percent)) << 9;
  if (direction == OutprizeDirection::IN) {
    low24 |= 0x20;
  }
  if (rain_enabled) {
    low24 |= 0x10;
  }
  low24 |= static_cast<uint8_t>(vent_command) & 0x0C;
  return low24;
}

bool RFBridgeComponent::send_outprize(uint32_t remote_id, uint8_t speed_percent, OutprizeDirection direction,
                                      bool rain_enabled, OutprizeVentCommand vent_command) {
  const uint32_t low24 = this->encode_outprize_low24(speed_percent, direction, rain_enabled, vent_command);
  ESP_LOGI(TAG, "Outprize TX remote_id=0x%06X low24=0x%06X", remote_id & 0xFFFFFF, low24 & 0xFFFFFF);
  return this->transmit_low24_(remote_id, low24);
}

bool RFBridgeComponent::transmit_low24_(uint32_t remote_id, uint32_t low24) {
  // TODO: implement OOK transmit using CC1101 async/direct TX.
  ESP_LOGW(TAG, "TX placeholder only; CC1101 transmit not implemented yet. remote=0x%06X low24=0x%06X",
           remote_id & 0xFFFFFF, low24 & 0xFFFFFF);
  return false;
}

}  // namespace rfbridge
}  // namespace esphome
