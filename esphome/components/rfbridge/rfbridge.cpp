#include "rfbridge.h"
#include "cc1101_regs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rfbridge {

static const char *const TAG = "rfbridge";

void RFBridgeComponent::setup() {
  ESP_LOGI(TAG, "Setting up RF Bridge v0.2.2...");

  if (this->cs_pin_ == nullptr || this->sck_pin_ == nullptr || this->mosi_pin_ == nullptr || this->miso_pin_ == nullptr) {
    ESP_LOGE(TAG, "Required SPI GPIO pins are not configured");
    this->mark_failed();
    return;
  }

  this->cs_pin_->setup();
  this->sck_pin_->setup();
  this->mosi_pin_->setup();
  this->miso_pin_->setup();

  this->cs_pin_->digital_write(true);
  this->sck_pin_->digital_write(false);
  this->mosi_pin_->digital_write(false);

  if (this->gdo0_pin_ != nullptr) {
    this->gdo0_pin_->setup();
  }
  if (this->gdo2_pin_ != nullptr) {
    this->gdo2_pin_->setup();
  }

  if (!this->cc1101_begin_()) {
    ESP_LOGE(TAG, "CC1101 bring-up failed; RF functions disabled, but ESPHome will remain online");
    return;
  }

  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();
  this->cc1101_configured_ = true;

  ESP_LOGI(TAG, "RF Bridge setup complete; CC1101 is in async RX mode");
}

void RFBridgeComponent::loop() {
  // Receive/decode path will be added after CC1101 bring-up is validated.
}

void RFBridgeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RF Bridge");
  LOG_PIN("  CS Pin: ", this->cs_pin_);
  LOG_PIN("  SCK Pin: ", this->sck_pin_);
  LOG_PIN("  MOSI Pin: ", this->mosi_pin_);
  LOG_PIN("  MISO Pin: ", this->miso_pin_);
  LOG_PIN("  GDO0 Pin: ", this->gdo0_pin_);
  LOG_PIN("  GDO2 Pin: ", this->gdo2_pin_);
  ESP_LOGCONFIG(TAG, "  CC1101 Detected: %s", YESNO(this->cc1101_detected_));
  ESP_LOGCONFIG(TAG, "  CC1101 Configured: %s", YESNO(this->cc1101_configured_));
  ESP_LOGCONFIG(TAG, "  CC1101 PARTNUM: 0x%02X", this->cc1101_partnum_);
  ESP_LOGCONFIG(TAG, "  CC1101 VERSION: 0x%02X", this->cc1101_version_);
}

bool RFBridgeComponent::cc1101_begin_() {
  ESP_LOGI(TAG, "Initializing CC1101 with native bit-banged SPI...");

  this->cc1101_reset_();

  this->cc1101_partnum_ = this->cc1101_read_status_(cc1101::PARTNUM);
  this->cc1101_version_ = this->cc1101_read_status_(cc1101::VERSION);

  ESP_LOGI(TAG, "CC1101 PARTNUM=0x%02X VERSION=0x%02X", this->cc1101_partnum_, this->cc1101_version_);

  // Common CC1101 values are PARTNUM=0x00 and VERSION=0x14. Some clones report
  // different VERSION values, so only treat all-zero/all-ones as a bus failure.
  if ((this->cc1101_partnum_ == 0x00 && this->cc1101_version_ == 0x00) ||
      (this->cc1101_partnum_ == 0xFF && this->cc1101_version_ == 0xFF)) {
    this->cc1101_detected_ = false;
    ESP_LOGE(TAG, "CC1101 did not respond correctly. Check power, CS, and SPI wiring.");
    return false;
  }

  this->cc1101_detected_ = true;
  ESP_LOGI(TAG, "CC1101 detected");
  return true;
}

void RFBridgeComponent::cc1101_reset_() {
  // CC1101 reset sequence using GPIO only. This intentionally avoids ESPHome's
  // SPI bus-lock path until the bridge has its own stable low-level driver.
  this->spi_deselect_();
  delayMicroseconds(5);
  this->spi_select_();
  delayMicroseconds(10);
  this->spi_deselect_();
  delayMicroseconds(40);
  this->cc1101_strobe_(cc1101::SRES);
  delay(1);
}

void RFBridgeComponent::cc1101_configure_ook_async_rx_() {
  ESP_LOGI(TAG, "Configuring CC1101 for 433.92 MHz OOK async RX...");

  this->cc1101_enter_idle_();

  this->cc1101_write_reg_(cc1101::FREQ2, 0x10);
  this->cc1101_write_reg_(cc1101::FREQ1, 0xB0);
  this->cc1101_write_reg_(cc1101::FREQ0, 0x71);

  this->cc1101_write_reg_(cc1101::IOCFG0, cc1101::GDO_SERIAL_DATA);
  this->cc1101_write_reg_(cc1101::IOCFG2, cc1101::GDO_HIGH_Z);
  this->cc1101_write_reg_(cc1101::PKTCTRL0, cc1101::PKT_ASYNC_SERIAL);

  this->cc1101_write_reg_(cc1101::FSCTRL1, 0x06);
  this->cc1101_write_reg_(cc1101::MDMCFG4, 0xF5);
  this->cc1101_write_reg_(cc1101::MDMCFG3, 0x83);
  this->cc1101_write_reg_(cc1101::MDMCFG2, 0x30);
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

  ESP_LOGI(TAG, "  IOCFG0   = 0x%02X", this->cc1101_read_reg_(cc1101::IOCFG0));
  ESP_LOGI(TAG, "  IOCFG2   = 0x%02X", this->cc1101_read_reg_(cc1101::IOCFG2));
  ESP_LOGI(TAG, "  PKTCTRL0 = 0x%02X", this->cc1101_read_reg_(cc1101::PKTCTRL0));
}

void RFBridgeComponent::cc1101_enter_rx_() {
  this->cc1101_strobe_(cc1101::SFRX);
  this->cc1101_strobe_(cc1101::SRX);
}

void RFBridgeComponent::cc1101_enter_idle_() { this->cc1101_strobe_(cc1101::SIDLE); }

void RFBridgeComponent::cc1101_write_reg_(uint8_t addr, uint8_t value) {
  this->spi_select_();
  this->spi_write_byte_(addr);
  this->spi_write_byte_(value);
  this->spi_deselect_();
}

uint8_t RFBridgeComponent::cc1101_read_reg_(uint8_t addr) {
  this->spi_select_();
  this->spi_write_byte_(addr | cc1101::READ_SINGLE);
  const uint8_t value = this->spi_read_byte_();
  this->spi_deselect_();
  return value;
}

uint8_t RFBridgeComponent::cc1101_read_status_(uint8_t addr) {
  this->spi_select_();
  this->spi_write_byte_(addr | cc1101::READ_BURST);
  const uint8_t value = this->spi_read_byte_();
  this->spi_deselect_();
  return value;
}

uint8_t RFBridgeComponent::cc1101_strobe_(uint8_t strobe) {
  this->spi_select_();
  const uint8_t status = this->spi_transfer_byte_(strobe);
  this->spi_deselect_();
  return status;
}

void RFBridgeComponent::cc1101_write_patable_(uint8_t value) {
  this->spi_select_();
  this->spi_write_byte_(cc1101::PATABLE);
  this->spi_write_byte_(value);
  this->spi_deselect_();
}

void RFBridgeComponent::spi_select_() {
  this->cs_pin_->digital_write(false);
  delayMicroseconds(2);
}

void RFBridgeComponent::spi_deselect_() {
  this->cs_pin_->digital_write(true);
  delayMicroseconds(2);
}

uint8_t RFBridgeComponent::spi_transfer_byte_(uint8_t value) {
  uint8_t result = 0;
  for (int8_t bit = 7; bit >= 0; bit--) {
    this->mosi_pin_->digital_write((value >> bit) & 0x01);
    delayMicroseconds(1);
    this->sck_pin_->digital_write(true);
    delayMicroseconds(1);
    result <<= 1;
    if (this->miso_pin_->digital_read()) {
      result |= 0x01;
    }
    this->sck_pin_->digital_write(false);
    delayMicroseconds(1);
  }
  return result;
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
  ESP_LOGW(TAG, "TX placeholder only; CC1101 transmit not implemented yet. remote=0x%06X low24=0x%06X",
           remote_id & 0xFFFFFF, low24 & 0xFFFFFF);
  return false;
}

}  // namespace rfbridge
}  // namespace esphome
