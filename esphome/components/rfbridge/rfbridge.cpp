#include "rfbridge.h"
#include "cc1101_regs.h"
#include "version.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rfbridge {

static const char *const TAG = "rfbridge";

void RFBridgeComponent::setup() {
  ESP_LOGI(TAG, "======================================");
  ESP_LOGI(TAG, "ESPHome RF Bridge v%s", RFBRIDGE_VERSION);
  ESP_LOGI(TAG, "Build: %s %s", RFBRIDGE_BUILD_DATE, RFBRIDGE_BUILD_TIME);
  ESP_LOGI(TAG, "Git: %s", RFBRIDGE_GIT_REF);
  ESP_LOGI(TAG, "======================================");

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

  this->cc1101_dump_registers_("post-reset/pre-config");

  this->cc1101_configure_ook_async_rx_();
  this->cc1101_dump_registers_("post-config/pre-rx");

  this->cc1101_enter_rx_();
  this->cc1101_dump_registers_("post-rx");
  this->cc1101_configured_ = true;

  this->rx_setup_();

  ESP_LOGI(TAG, "RF Bridge setup complete; CC1101 is in async RX mode and listening");
}

void RFBridgeComponent::loop() {
  this->rx_poll_();
}

void RFBridgeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RF Bridge");
  ESP_LOGCONFIG(TAG, "  Firmware Version: %s", RFBRIDGE_VERSION);
  ESP_LOGCONFIG(TAG, "  Build: %s %s", RFBRIDGE_BUILD_DATE, RFBRIDGE_BUILD_TIME);
  ESP_LOGCONFIG(TAG, "  Git: %s", RFBRIDGE_GIT_REF);
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
  ESP_LOGCONFIG(TAG, "  RX Enabled: %s", YESNO(this->rx_enabled_));
  ESP_LOGCONFIG(TAG, "  RX Packets Seen: %u", this->rx_packets_seen_);
  ESP_LOGCONFIG(TAG, "  RX Edges Seen: %u", this->rx_edges_seen_);
  ESP_LOGCONFIG(TAG, "  RX Overruns: %u", this->rx_overruns_);
  ESP_LOGCONFIG(TAG, "  Discarded Partials: %u", this->rx_discarded_partials_);
  ESP_LOGCONFIG(TAG, "  Last Packet: %u edges, %u us, RSSI %d dBm", this->rx_last_packet_edges_,
                this->rx_last_packet_duration_us_, this->rx_last_rssi_dbm_);
  ESP_LOGCONFIG(TAG, "  Last Packet Gaps: min=%u us avg=%u us max=%u us", this->rx_last_min_gap_us_,
                this->rx_last_avg_gap_us_, this->rx_last_max_gap_us_);
}

bool RFBridgeComponent::cc1101_begin_() {
  ESP_LOGI(TAG, "Initializing CC1101 with native bit-banged SPI...");

  this->cc1101_reset_();
  ESP_LOGI(TAG, "CC1101 reset complete");

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
  ESP_LOGI(TAG, "Detected CC1101 (PARTNUM=0x%02X VERSION=0x%02X)", this->cc1101_partnum_, this->cc1101_version_);
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
  ESP_LOGI(TAG, "Applying readback-matched known-good sniffer profile");

  this->cc1101_enter_idle_();

  this->cc1101_write_reg_(cc1101::FREQ2, 0x10);
  this->cc1101_write_reg_(cc1101::FREQ1, 0xB0);
  this->cc1101_write_reg_(cc1101::FREQ0, 0x71);

  this->cc1101_write_reg_(cc1101::IOCFG0, cc1101::GDO_SERIAL_DATA);
  this->cc1101_write_reg_(cc1101::IOCFG2, cc1101::GDO_IOCFG2_KNOWN_GOOD);
  this->cc1101_write_reg_(cc1101::PKTCTRL0, cc1101::PKTCTRL0_KNOWN_GOOD);
  this->cc1101_write_reg_(cc1101::PKTLEN, 0xFF);

  this->cc1101_write_reg_(cc1101::FSCTRL1, 0x06);
  // Match the known-good diagnostic sniffer configuration:
  //   setBitRate(2.0) + setRxBandwidth(58.0)
  // CHANBW_E=3, CHANBW_M=3, DRATE_E=6 => MDMCFG4=0xF6
  // DRATE_M=0x43 gives ~2.0 kbps with a 26 MHz crystal.
  this->cc1101_write_reg_(cc1101::MDMCFG4, 0xF6);
  this->cc1101_write_reg_(cc1101::MDMCFG3, 0x43);
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
  ESP_LOGI(TAG, "  PKTLEN   = 0x%02X", this->cc1101_read_reg_(cc1101::PKTLEN));
  ESP_LOGI(TAG, "  PKTCTRL0 = 0x%02X", this->cc1101_read_reg_(cc1101::PKTCTRL0));
}

void RFBridgeComponent::cc1101_log_register_(const char *name, uint8_t addr, int expected) {
  const uint8_t value = this->cc1101_read_reg_(addr);
  if (expected >= 0) {
    ESP_LOGI(TAG, "  %-8s [0x%02X] = 0x%02X  expected=0x%02X  %s", name, addr, value, expected & 0xFF,
             value == (expected & 0xFF) ? "OK" : "MISMATCH");
  } else {
    ESP_LOGI(TAG, "  %-8s [0x%02X] = 0x%02X", name, addr, value);
  }
}

void RFBridgeComponent::cc1101_dump_registers_(const char *stage) {
  ESP_LOGI(TAG, "CC1101 register dump (%s):", stage);
  this->cc1101_log_register_("IOCFG2", cc1101::IOCFG2, cc1101::GDO_IOCFG2_KNOWN_GOOD);
  this->cc1101_log_register_("IOCFG1", cc1101::IOCFG1);
  this->cc1101_log_register_("IOCFG0", cc1101::IOCFG0, cc1101::GDO_SERIAL_DATA);
  this->cc1101_log_register_("FIFOTHR", cc1101::FIFOTHR);
  this->cc1101_log_register_("SYNC1", cc1101::SYNC1);
  this->cc1101_log_register_("SYNC0", cc1101::SYNC0);
  this->cc1101_log_register_("PKTLEN", cc1101::PKTLEN, 0xFF);
  this->cc1101_log_register_("PKTCTRL1", cc1101::PKTCTRL1);
  this->cc1101_log_register_("PKTCTRL0", cc1101::PKTCTRL0, cc1101::PKTCTRL0_KNOWN_GOOD);
  this->cc1101_log_register_("ADDR", cc1101::ADDR);
  this->cc1101_log_register_("CHANNR", cc1101::CHANNR);
  this->cc1101_log_register_("FSCTRL1", cc1101::FSCTRL1, 0x06);
  this->cc1101_log_register_("FSCTRL0", cc1101::FSCTRL0);
  this->cc1101_log_register_("FREQ2", cc1101::FREQ2, 0x10);
  this->cc1101_log_register_("FREQ1", cc1101::FREQ1, 0xB0);
  this->cc1101_log_register_("FREQ0", cc1101::FREQ0, 0x71);
  this->cc1101_log_register_("MDMCFG4", cc1101::MDMCFG4, 0xF6);
  this->cc1101_log_register_("MDMCFG3", cc1101::MDMCFG3, 0x43);
  this->cc1101_log_register_("MDMCFG2", cc1101::MDMCFG2, 0x30);
  this->cc1101_log_register_("MDMCFG1", cc1101::MDMCFG1, 0x22);
  this->cc1101_log_register_("MDMCFG0", cc1101::MDMCFG0, 0xF8);
  this->cc1101_log_register_("DEVIATN", cc1101::DEVIATN);
  this->cc1101_log_register_("MCSM2", cc1101::MCSM2);
  this->cc1101_log_register_("MCSM1", cc1101::MCSM1);
  this->cc1101_log_register_("MCSM0", cc1101::MCSM0, 0x18);
  this->cc1101_log_register_("FOCCFG", cc1101::FOCCFG, 0x16);
  this->cc1101_log_register_("BSCFG", cc1101::BSCFG);
  this->cc1101_log_register_("AGCCTRL2", cc1101::AGCCTRL2, 0x04);
  this->cc1101_log_register_("AGCCTRL1", cc1101::AGCCTRL1, 0x00);
  this->cc1101_log_register_("AGCCTRL0", cc1101::AGCCTRL0, 0x91);
  this->cc1101_log_register_("WOREVT1", cc1101::WOREVT1);
  this->cc1101_log_register_("WOREVT0", cc1101::WOREVT0);
  this->cc1101_log_register_("WORCTRL", cc1101::WORCTRL);
  this->cc1101_log_register_("FREND1", cc1101::FREND1, 0x56);
  this->cc1101_log_register_("FREND0", cc1101::FREND0, 0x11);
  this->cc1101_log_register_("FSCAL3", cc1101::FSCAL3, 0xE9);
  this->cc1101_log_register_("FSCAL2", cc1101::FSCAL2, 0x2A);
  this->cc1101_log_register_("FSCAL1", cc1101::FSCAL1, 0x00);
  this->cc1101_log_register_("FSCAL0", cc1101::FSCAL0, 0x1F);
  this->cc1101_log_register_("TEST2", cc1101::TEST2, 0x81);
  this->cc1101_log_register_("TEST1", cc1101::TEST1, 0x35);
  this->cc1101_log_register_("TEST0", cc1101::TEST0, 0x09);

  ESP_LOGI(TAG, "CC1101 status dump (%s):", stage);
  ESP_LOGI(TAG, "  RSSI     [0x%02X] = 0x%02X (%d dBm)", cc1101::RSSI, this->cc1101_read_status_(cc1101::RSSI),
           this->cc1101_read_rssi_dbm_());
  ESP_LOGI(TAG, "  MARCSTATE[0x%02X] = 0x%02X", cc1101::MARCSTATE, this->cc1101_read_status_(cc1101::MARCSTATE));
  ESP_LOGI(TAG, "  PKTSTATUS[0x%02X] = 0x%02X", cc1101::PKTSTATUS, this->cc1101_read_status_(cc1101::PKTSTATUS));
  ESP_LOGI(TAG, "  RXBYTES  [0x%02X] = 0x%02X", cc1101::RXBYTES, this->cc1101_read_status_(cc1101::RXBYTES));
}


void RFBridgeComponent::cc1101_enter_rx_() {
  ESP_LOGI(TAG, "Entering CC1101 RX mode; listening for RF activity...");
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


int16_t RFBridgeComponent::cc1101_read_rssi_dbm_() {
  const uint8_t raw = this->cc1101_read_status_(cc1101::RSSI);
  int16_t rssi = raw;
  if (rssi >= 128) {
    rssi = ((rssi - 256) / 2) - 74;
  } else {
    rssi = (rssi / 2) - 74;
  }
  return rssi;
}

void RFBridgeComponent::rx_setup_() {
  if (this->gdo0_pin_ == nullptr) {
    ESP_LOGW(TAG, "GDO0 pin is not configured; RX edge capture disabled");
    this->rx_enabled_ = false;
    return;
  }

  this->rx_enabled_ = true;
  this->rx_have_level_ = false;
  this->rx_edge_count_ = 0;
  this->rx_packets_seen_ = 0;
  this->rx_edges_seen_ = 0;
  this->rx_overruns_ = 0;
  this->rx_discarded_partials_ = 0;
  ESP_LOGI(TAG, "RX pipeline ready: polling GDO0 for async OOK edges");
  ESP_LOGI(TAG, "RX thresholds: min_edges=%u min_duration=%u us packet_gap=%u us stale_partial=%u us",
           RX_MIN_EDGES, RX_MIN_PACKET_US, RX_PACKET_GAP_US, RX_STALE_PARTIAL_US);
}

void RFBridgeComponent::rx_poll_() {
  if (!this->rx_enabled_ || this->gdo0_pin_ == nullptr) {
    return;
  }

  const uint32_t now_us = micros();
  const bool level = this->gdo0_pin_->digital_read();

  if (!this->rx_have_level_) {
    this->rx_reset_packet_(now_us, level);
    this->rx_have_level_ = true;
    return;
  }

  if (level != this->rx_last_level_) {
    this->rx_record_edge_(now_us, level);
    return;
  }

  const uint32_t quiet_us = now_us - this->rx_last_edge_us_;

  if (this->rx_edge_count_ >= RX_MIN_EDGES && quiet_us > RX_PACKET_GAP_US) {
    this->rx_finish_packet_(now_us);
    this->rx_reset_packet_(now_us, level);
    return;
  }

  if (this->rx_edge_count_ > 0 && this->rx_edge_count_ < RX_MIN_EDGES && quiet_us > RX_STALE_PARTIAL_US) {
    this->rx_discarded_partials_++;
    ESP_LOGV(TAG, "Discarding stale RF partial: edges=%u quiet=%u us", this->rx_edge_count_, quiet_us);
    this->rx_reset_packet_(now_us, level);
  }
}

void RFBridgeComponent::rx_record_edge_(uint32_t now_us, bool level) {
  const uint32_t delta = now_us - this->rx_last_edge_us_;
  this->rx_last_edge_us_ = now_us;
  this->rx_last_level_ = level;
  this->rx_edges_seen_++;

  if (this->rx_edge_count_ == 0) {
    this->rx_packet_start_us_ = now_us;
  }

  if (this->rx_edge_count_ < RX_MAX_EDGES) {
    this->rx_edges_[this->rx_edge_count_++] = delta > 0xFFFF ? 0xFFFF : static_cast<uint16_t>(delta);
  } else {
    this->rx_overruns_++;
  }

  const uint32_t now_ms = millis();
  ESP_LOGVV(TAG, "RF edge #%u delta=%u us level=%d", this->rx_edge_count_, delta, level);

  if ((now_ms - this->rx_last_activity_log_ms_) > 1000) {
    this->rx_last_activity_log_ms_ = now_ms;
    ESP_LOGD(TAG, "RF activity detected on GDO0; edges_seen=%u current_edges=%u",
             this->rx_edges_seen_, this->rx_edge_count_);
  }
}

void RFBridgeComponent::rx_finish_packet_(uint32_t now_us) {
  const uint32_t duration_us = now_us - this->rx_packet_start_us_;
  if (duration_us < RX_MIN_PACKET_US) {
    return;
  }

  uint32_t sum_gap = 0;
  uint16_t min_gap = 0xFFFF;
  uint16_t max_gap = 0;
  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    const uint16_t gap = this->rx_edges_[i];
    if (gap < min_gap) min_gap = gap;
    if (gap > max_gap) max_gap = gap;
    sum_gap += gap;
  }

  this->rx_packets_seen_++;
  this->rx_last_packet_edges_ = this->rx_edge_count_;
  this->rx_last_packet_duration_us_ = duration_us;
  this->rx_last_min_gap_us_ = min_gap == 0xFFFF ? 0 : min_gap;
  this->rx_last_max_gap_us_ = max_gap;
  this->rx_last_avg_gap_us_ = this->rx_edge_count_ > 0 ? static_cast<uint16_t>(sum_gap / this->rx_edge_count_) : 0;
  this->rx_last_rssi_dbm_ = this->cc1101_read_rssi_dbm_();

  ESP_LOGI(TAG, "RF packet candidate #%u: edges=%u duration=%u us rssi=%d dBm gaps[min/avg/max]=%u/%u/%u us",
           this->rx_packets_seen_, this->rx_last_packet_edges_, this->rx_last_packet_duration_us_,
           this->rx_last_rssi_dbm_, this->rx_last_min_gap_us_, this->rx_last_avg_gap_us_,
           this->rx_last_max_gap_us_);
}

void RFBridgeComponent::rx_reset_packet_(uint32_t now_us, bool level) {
  this->rx_edge_count_ = 0;
  this->rx_packet_start_us_ = now_us;
  this->rx_last_edge_us_ = now_us;
  this->rx_last_level_ = level;
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
