#include <algorithm>
#include "rfbridge.h"
#include "cc1101_regs.h"
#include "version.h"
#include "esphome/core/log.h"
#include <cstdio>

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
  if (this->stx882_data_pin_ != nullptr) {
    this->stx882_data_pin_->setup();
    this->stx882_data_pin_->digital_write(false);
  }
  if (this->srx882_data_pin_ != nullptr) {
    this->srx882_data_pin_->setup();
  }
  if (this->srx882_enable_pin_ != nullptr) {
    this->srx882_enable_pin_->setup();
    // SRX882/SRX882S CS is active-high: HIGH = receive, LOW = standby.
    this->srx882_enable_pin_->digital_write(true);
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

  ESP_LOGI(TAG, "Registered codec: %s", this->outprize_codec_.capability_summary().c_str());
  ESP_LOGI(TAG, "RF Bridge setup complete; CC1101 is in async RX mode and listening");
}

void RFBridgeComponent::loop() {
  if (this->tx_carrier_active_) {
    this->tx_carrier_loop_();
    return;
  }
  if (this->sequence_active_ && static_cast<int32_t>(millis() - this->sequence_until_ms_) > 0) {
    this->sequence_active_ = false;
    snprintf(this->sequence_last_summary_, sizeof(this->sequence_last_summary_), "%u frame(s) captured", this->sequence_count_);
    ESP_LOGI(TAG, "RF_SEQUENCE_LEARN complete frames=%u", this->sequence_count_);
  }
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
  LOG_PIN("  STX882 DATA Pin: ", this->stx882_data_pin_);
  LOG_PIN("  SRX882 DATA Pin: ", this->srx882_data_pin_);
  LOG_PIN("  SRX882 Enable Pin: ", this->srx882_enable_pin_);
  ESP_LOGCONFIG(TAG, "  CC1101 Detected: %s", YESNO(this->cc1101_detected_));
  ESP_LOGCONFIG(TAG, "  CC1101 Configured: %s", YESNO(this->cc1101_configured_));
  ESP_LOGCONFIG(TAG, "  CC1101 PARTNUM: 0x%02X", this->cc1101_partnum_);
  ESP_LOGCONFIG(TAG, "  CC1101 VERSION: 0x%02X", this->cc1101_version_);
  ESP_LOGCONFIG(TAG, "  RX Enabled: %s", YESNO(this->rx_enabled_));
  ESP_LOGCONFIG(TAG, "  RX Mode: RSSI-gated fixed-window verified Outprize decoder");
  ESP_LOGCONFIG(TAG, "  TX Mode: CC1101 async OOK plus optional direct STX882 ASK/OOK backend");
  ESP_LOGCONFIG(TAG, "  STX882 Available: %s", YESNO(this->stx882_data_pin_ != nullptr));
  ESP_LOGCONFIG(TAG, "  SRX882 Available: %s", YESNO(this->srx882_data_pin_ != nullptr));
  ESP_LOGCONFIG(TAG, "  Diagnostic Logging: %s", YESNO(this->diagnostic_logging_));
  ESP_LOGCONFIG(TAG, "  Codec: %s", this->outprize_codec_.capability_summary().c_str());
  ESP_LOGCONFIG(TAG, "  Capabilities: %s", this->get_bridge_capabilities().c_str());
  ESP_LOGCONFIG(TAG, "  Learned Outprize Frame: %s", YESNO(this->outprize_learned_valid_));
  ESP_LOGCONFIG(TAG, "  Outprize Remote ID / 11-bit prefix: 0x%03X", this->outprize_remote_id_ & 0x7FF);
  ESP_LOGCONFIG(TAG, "  RX RSSI Arm Threshold: %d dBm", RX_RSSI_ARM_DBM);
  ESP_LOGCONFIG(TAG, "  RX Capture Window: %u us", RX_CAPTURE_WINDOW_US);
  ESP_LOGCONFIG(TAG, "  RX Packets Seen: %u", this->rx_packets_seen_);
  ESP_LOGCONFIG(TAG, "  RX Edges Seen: %u", this->rx_edges_seen_);
  ESP_LOGCONFIG(TAG, "  RX Overruns: %u", this->rx_overruns_);
  ESP_LOGCONFIG(TAG, "  Discarded Partials: %u", this->rx_discarded_partials_);
  ESP_LOGCONFIG(TAG, "  Last Packet: %u edges, %u us, RSSI %d dBm trigger %d dBm", this->rx_last_packet_edges_,
                this->rx_last_packet_duration_us_, this->rx_last_rssi_dbm_, this->rx_last_trigger_rssi_dbm_);
  ESP_LOGCONFIG(TAG, "  Last Packet Gaps: min=%u us avg=%u us max=%u us", this->rx_last_min_gap_us_,
                this->rx_last_avg_gap_us_, this->rx_last_max_gap_us_);
  ESP_LOGCONFIG(TAG, "  Last Fingerprint: 0x%08X filtered=%u unique_bins=%u", this->rx_last_fingerprint_,
                this->rx_last_filtered_edges_, this->rx_last_unique_bins_);
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
  this->cc1101_write_reg_(cc1101::MDMCFG2, this->tx_mdmcfg2_value_);
  this->cc1101_write_reg_(cc1101::MDMCFG1, 0x22);
  this->cc1101_write_reg_(cc1101::MDMCFG0, 0xF8);
  this->cc1101_write_reg_(cc1101::MCSM0, 0x18);
  this->cc1101_write_reg_(cc1101::FOCCFG, 0x16);
  this->cc1101_write_reg_(cc1101::AGCCTRL2, 0x04);
  this->cc1101_write_reg_(cc1101::AGCCTRL1, 0x00);
  this->cc1101_write_reg_(cc1101::AGCCTRL0, 0x91);
  this->cc1101_write_reg_(cc1101::FREND1, 0x56);
  this->cc1101_write_reg_(cc1101::FREND0, this->tx_frend0_value_);
  this->cc1101_write_reg_(cc1101::FSCAL3, 0xE9);
  this->cc1101_write_reg_(cc1101::FSCAL2, 0x2A);
  this->cc1101_write_reg_(cc1101::FSCAL1, 0x00);
  this->cc1101_write_reg_(cc1101::FSCAL0, 0x1F);
  this->cc1101_write_reg_(cc1101::TEST2, 0x81);
  this->cc1101_write_reg_(cc1101::TEST1, 0x35);
  this->cc1101_write_reg_(cc1101::TEST0, 0x09);

  this->cc1101_write_patable_(this->tx_pa_value_);

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
    ESP_LOGW(TAG, "GDO0 pin is not configured; RX capture disabled");
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
  this->rx_last_rssi_poll_ms_ = 0;
  this->rx_last_capture_ms_ = 0;
  ESP_LOGI(TAG, "RX pipeline ready: RSSI-gated fixed-window RF pulse analyzer");
  ESP_LOGI(TAG, "RX thresholds: arm_rssi=%d dBm capture_window=%u us cooldown=%u ms min_edges=%u",
           RX_RSSI_ARM_DBM, RX_CAPTURE_WINDOW_US, RX_CAPTURE_COOLDOWN_MS, RX_MIN_EDGES);
}

void RFBridgeComponent::rx_poll_() {
  if (!this->rx_enabled_ || this->gdo0_pin_ == nullptr) {
    return;
  }

  const uint32_t now_ms = millis();
  if ((now_ms - this->rx_last_rssi_poll_ms_) < RX_RSSI_POLL_INTERVAL_MS) {
    return;
  }
  this->rx_last_rssi_poll_ms_ = now_ms;

  const int16_t rssi = this->cc1101_read_rssi_dbm_();
  this->rx_last_rssi_dbm_ = rssi;

  if (rssi < RX_RSSI_ARM_DBM) {
    return;
  }

  const uint32_t cooldown_ms = this->sequence_active_ ? 25 : RX_CAPTURE_COOLDOWN_MS;
  if ((now_ms - this->rx_last_capture_ms_) < cooldown_ms) {
    ESP_LOGV(TAG, "RSSI %d dBm crossed threshold during cooldown", rssi);
    return;
  }

  this->rx_last_capture_ms_ = now_ms;
  if (this->diagnostic_logging_) {
    ESP_LOGI(TAG, "RSSI trigger: %d dBm >= %d dBm; capturing GDO0 for %u us",
             rssi, RX_RSSI_ARM_DBM, RX_CAPTURE_WINDOW_US);
  }
  this->rx_capture_window_(rssi);
}

void RFBridgeComponent::rx_capture_window_(int16_t trigger_rssi_dbm) {
  this->rx_edge_count_ = 0;
  const uint32_t start_us = micros();
  uint32_t last_edge_us = start_us;
  bool last_level = this->gdo0_pin_->digital_read();
  this->rx_capture_initial_level_ = last_level ? 1 : 0;

  while ((micros() - start_us) < RX_CAPTURE_WINDOW_US) {
    const bool level = this->gdo0_pin_->digital_read();
    if (level == last_level) {
      continue;
    }

    const uint32_t now_us = micros();
    const uint32_t delta = now_us - last_edge_us;
    last_edge_us = now_us;
    last_level = level;
    this->rx_edges_seen_++;

    if (this->rx_edge_count_ < RX_MAX_EDGES) {
      this->rx_edges_[this->rx_edge_count_] = delta > 0xFFFF ? 0xFFFF : static_cast<uint16_t>(delta);
      this->rx_levels_[this->rx_edge_count_] = last_level ? 1 : 0;
      this->rx_edge_count_++;
    } else {
      this->rx_overruns_++;
    }
  }

  this->rx_finish_capture_(start_us, micros(), trigger_rssi_dbm);
}

void RFBridgeComponent::rx_finish_capture_(uint32_t start_us, uint32_t end_us, int16_t trigger_rssi_dbm) {
  const uint32_t duration_us = end_us - start_us;
  this->rx_last_trigger_rssi_dbm_ = trigger_rssi_dbm;

  if (this->rx_edge_count_ < RX_MIN_EDGES || duration_us < RX_MIN_PACKET_US) {
    this->rx_discarded_partials_++;
    ESP_LOGI(TAG, "RF capture discarded: edges=%u duration=%u us trigger_rssi=%d dBm",
             this->rx_edge_count_, duration_us, trigger_rssi_dbm);
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

  if (this->diagnostic_logging_) {
    ESP_LOGI(TAG, "RF RSSI-gated capture #%u: edges=%u duration=%u us trigger_rssi=%d dBm end_rssi=%d dBm gaps[min/avg/max]=%u/%u/%u us",
             this->rx_packets_seen_, this->rx_last_packet_edges_, this->rx_last_packet_duration_us_,
             trigger_rssi_dbm, this->rx_last_rssi_dbm_, this->rx_last_min_gap_us_, this->rx_last_avg_gap_us_,
             this->rx_last_max_gap_us_);
  }

  this->rx_last_outprize_like_ = false;
  const bool decoded = this->rx_log_outprize_decode_(this->rx_packets_seen_);
  this->sequence_store_current_capture_(this->rx_packets_seen_, decoded);

  if (this->diagnostic_logging_) {
    this->rx_log_full_capture_timeline_(this->rx_packets_seen_);
    this->rx_log_raw_timings_(this->rx_packets_seen_);
    this->rx_log_pulse_histogram_(this->rx_packets_seen_);
    this->rx_log_protocol_analysis_(this->rx_packets_seen_);
  } else if (!decoded && !this->rx_last_outprize_like_) {
    ESP_LOGD(TAG, "RF capture #%u ignored: protocol=unknown edges=%u rssi=%d dBm",
             this->rx_packets_seen_, this->rx_last_packet_edges_, trigger_rssi_dbm);
  }
}

void RFBridgeComponent::rx_log_raw_timings_(uint32_t capture_no) {
  ESP_LOGI(TAG, "Capture #%u raw edge timing deltas (us), count=%u:", capture_no, this->rx_edge_count_);

  char line[192];
  for (uint16_t i = 0; i < this->rx_edge_count_; i += 16) {
    int offset = snprintf(line, sizeof(line), "  [%03u-%03u]", i,
                          static_cast<unsigned>(i + 15 < this->rx_edge_count_ ? i + 15 : this->rx_edge_count_ - 1));
    for (uint16_t j = i; j < this->rx_edge_count_ && j < i + 16 && offset > 0 && offset < static_cast<int>(sizeof(line)); j++) {
      offset += snprintf(line + offset, sizeof(line) - offset, " %u", this->rx_edges_[j]);
    }
    ESP_LOGI(TAG, "%s", line);
  }
}


void RFBridgeComponent::rx_log_full_capture_timeline_(uint32_t capture_no) {
  ESP_LOGI(TAG, "===== RF_FULL_CAPTURE_TIMELINE =====");
  ESP_LOGI(TAG, "Capture #%u trigger_rssi=%d dBm end_rssi=%d dBm initial_gdo0=%u edge_count=%u duration=%u us",
           capture_no, this->rx_last_trigger_rssi_dbm_, this->rx_last_rssi_dbm_,
           this->rx_capture_initial_level_, this->rx_edge_count_, this->rx_last_packet_duration_us_);

  const bool have_learned_for_capture = this->outprize_learned_valid_ && this->outprize_learned_capture_no_ == capture_no;
  const uint16_t data_start = have_learned_for_capture ? this->outprize_learned_start_index_ : 0xFFFF;
  const uint16_t data_stop = have_learned_for_capture ? this->outprize_learned_stop_index_ : 0xFFFF;
  if (have_learned_for_capture) {
    ESP_LOGI(TAG, "Markers: RSSI_TRIGGER=t0 first_edge_delay=%u us decoder_start_edge=%u decoder_stop_edge=%u learned_full35=0x%09llX",
             this->rx_edge_count_ > 0 ? this->rx_edges_[0] : 0, data_start, data_stop,
             static_cast<unsigned long long>(this->outprize_learned_full35_));
  } else {
    ESP_LOGI(TAG, "Markers: RSSI_TRIGGER=t0 first_edge_delay=%u us decoder_start_edge=unknown decoder_stop_edge=unknown",
             this->rx_edge_count_ > 0 ? this->rx_edges_[0] : 0);
  }

  // Print cumulative edge timing with the GDO0 level after each transition.
  // This is the highest-fidelity view currently available from the ESP-side
  // RSSI-gated capture path and helps distinguish real preamble/header edges
  // from decoder alignment artifacts.
  uint32_t t = 0;
  for (uint16_t row = 0; row < this->rx_edge_count_ && row < 96; row += 8) {
    char line[260];
    uint16_t pos = 0;
    const uint16_t end = (row + 7 < this->rx_edge_count_) ? row + 7 : this->rx_edge_count_ - 1;
    pos += snprintf(line + pos, sizeof(line) - pos, "  edges[%03u-%03u]", row, end);
    for (uint16_t i = row; i <= end; i++) {
      // Recompute cumulative time for this row start to avoid storing another array.
      if (i == row) {
        t = 0;
        for (uint16_t j = 0; j <= i; j++) t += this->rx_edges_[j];
      } else {
        t += this->rx_edges_[i];
      }
      const bool is_start = have_learned_for_capture && i == data_start;
      const bool is_stop = have_learned_for_capture && i == data_stop;
      pos += snprintf(line + pos, sizeof(line) - pos, " %s%u:%u/%u%s",
                      is_start ? "<" : "", i, this->rx_edges_[i], t, is_stop ? ">" : "");
    }
    ESP_LOGI(TAG, "%s", line);
  }
  if (this->rx_edge_count_ > 96) {
    ESP_LOGI(TAG, "  timeline truncated at 96 of %u edges", this->rx_edge_count_);
  }

  if (have_learned_for_capture) {
    char pre[220];
    uint16_t p = 0;
    const uint16_t pre_start = data_start >= 8 ? data_start - 8 : 0;
    p += snprintf(pre + p, sizeof(pre) - p, "Pre-data edge deltas [%u-%u]:", pre_start, data_start);
    for (uint16_t i = pre_start; i <= data_start && i < this->rx_edge_count_; i++) {
      p += snprintf(pre + p, sizeof(pre) - p, " %u", this->rx_edges_[i]);
    }
    ESP_LOGI(TAG, "%s", pre);
    ESP_LOGI(TAG, "Note: this is an RSSI-triggered capture; true RF activity before RSSI_TRIGGER is not observable without a continuous pre-trigger buffer.");
  }
  ESP_LOGI(TAG, "====================================");
}

void RFBridgeComponent::rx_log_pulse_histogram_(uint32_t capture_no) {
  uint16_t hist[RX_HIST_BIN_COUNT]{};

  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    uint16_t bin = static_cast<uint16_t>((this->rx_edges_[i] + (RX_HIST_BIN_US / 2)) / RX_HIST_BIN_US);
    if (bin >= RX_HIST_BIN_COUNT) {
      bin = RX_HIST_BIN_COUNT - 1;
    }
    hist[bin]++;
  }

  ESP_LOGI(TAG, "Capture #%u pulse width histogram, %u us bins:", capture_no, RX_HIST_BIN_US);

  char line[192];
  int offset = snprintf(line, sizeof(line), "  bins:");
  uint8_t entries_on_line = 0;
  for (uint16_t bin = 0; bin < RX_HIST_BIN_COUNT; bin++) {
    if (hist[bin] == 0) {
      continue;
    }

    const uint16_t center = static_cast<uint16_t>(bin * RX_HIST_BIN_US);
    const int written = snprintf(line + offset, sizeof(line) - offset, " %u:%u", center, hist[bin]);
    if (written <= 0 || written >= static_cast<int>(sizeof(line) - offset) || ++entries_on_line >= 10) {
      ESP_LOGI(TAG, "%s", line);
      offset = snprintf(line, sizeof(line), "  bins:");
      entries_on_line = 0;
      if (written > 0 && written < static_cast<int>(sizeof(line) - offset)) {
        offset += snprintf(line + offset, sizeof(line) - offset, " %u:%u", center, hist[bin]);
        entries_on_line = 1;
      }
    } else {
      offset += written;
    }
  }
  if (entries_on_line > 0) {
    ESP_LOGI(TAG, "%s", line);
  }

  ESP_LOGI(TAG, "Capture #%u dominant pulse buckets:", capture_no);
  bool used[RX_HIST_BIN_COUNT]{};
  for (uint8_t rank = 0; rank < 8; rank++) {
    uint16_t best_bin = 0;
    uint16_t best_count = 0;
    for (uint16_t bin = 0; bin < RX_HIST_BIN_COUNT; bin++) {
      if (!used[bin] && hist[bin] > best_count) {
        best_count = hist[bin];
        best_bin = bin;
      }
    }
    if (best_count == 0) {
      break;
    }
    used[best_bin] = true;
    ESP_LOGI(TAG, "  #%u: ~%u us  count=%u", rank + 1, best_bin * RX_HIST_BIN_US, best_count);
  }
}


uint16_t RFBridgeComponent::rx_normalize_pulse_(uint16_t pulse_us) const {
  if (pulse_us < RX_ANALYSIS_MIN_US || pulse_us > RX_ANALYSIS_MAX_US) {
    return 0;
  }
  return static_cast<uint16_t>(((pulse_us + (RX_ANALYSIS_BIN_US / 2)) / RX_ANALYSIS_BIN_US) * RX_ANALYSIS_BIN_US);
}

uint32_t RFBridgeComponent::rx_capture_fingerprint_() const {
  // FNV-1a over the normalized pulse stream. This is not a protocol CRC; it is
  // just a quick signature for comparing repeated captures.
  uint32_t hash = 2166136261UL;
  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    const uint16_t norm = this->rx_normalize_pulse_(this->rx_edges_[i]);
    if (norm == 0) {
      continue;
    }
    hash ^= static_cast<uint8_t>(norm & 0xFF);
    hash *= 16777619UL;
    hash ^= static_cast<uint8_t>((norm >> 8) & 0xFF);
    hash *= 16777619UL;
  }
  return hash;
}

void RFBridgeComponent::rx_log_protocol_analysis_(uint32_t capture_no) {
  uint16_t filtered_count = 0;
  uint16_t ignored_short = 0;
  uint16_t ignored_long = 0;
  uint16_t unique_bins = 0;
  uint16_t hist[RX_ANALYSIS_BIN_COUNT]{};

  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    const uint16_t pulse = this->rx_edges_[i];
    if (pulse < RX_ANALYSIS_MIN_US) {
      ignored_short++;
      continue;
    }
    if (pulse > RX_ANALYSIS_MAX_US) {
      ignored_long++;
      continue;
    }
    const uint16_t norm = this->rx_normalize_pulse_(pulse);
    const uint16_t bin = norm / RX_ANALYSIS_BIN_US;
    if (bin < RX_ANALYSIS_BIN_COUNT) {
      if (hist[bin] == 0) {
        unique_bins++;
      }
      hist[bin]++;
      filtered_count++;
    }
  }

  const uint32_t fingerprint = this->rx_capture_fingerprint_();
  this->rx_last_fingerprint_ = fingerprint;
  this->rx_last_filtered_edges_ = filtered_count;
  this->rx_last_unique_bins_ = unique_bins;

  ESP_LOGI(TAG, "Capture #%u protocol analysis: fingerprint=0x%08X filtered=%u ignored_short=%u ignored_long=%u unique_bins=%u",
           capture_no, fingerprint, filtered_count, ignored_short, ignored_long, unique_bins);

  char stream[192];
  int offset = snprintf(stream, sizeof(stream), "  normalized:");
  uint8_t on_line = 0;
  uint16_t emitted = 0;
  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    const uint16_t norm = this->rx_normalize_pulse_(this->rx_edges_[i]);
    if (norm == 0) {
      continue;
    }
    const int written = snprintf(stream + offset, sizeof(stream) - offset, " %u", norm);
    if (written <= 0 || written >= static_cast<int>(sizeof(stream) - offset) || ++on_line >= 16) {
      ESP_LOGI(TAG, "%s", stream);
      offset = snprintf(stream, sizeof(stream), "  normalized:");
      on_line = 0;
      if (written > 0 && written < static_cast<int>(sizeof(stream) - offset)) {
        offset += snprintf(stream + offset, sizeof(stream) - offset, " %u", norm);
        on_line = 1;
      }
    } else {
      offset += written;
    }
    emitted++;
    if (emitted >= RX_ANALYSIS_MAX_PRINTED_SYMBOLS) {
      break;
    }
  }
  if (on_line > 0) {
    ESP_LOGI(TAG, "%s", stream);
  }
  if (filtered_count > RX_ANALYSIS_MAX_PRINTED_SYMBOLS) {
    ESP_LOGI(TAG, "  normalized: ... truncated at %u of %u symbols", RX_ANALYSIS_MAX_PRINTED_SYMBOLS, filtered_count);
  }

  ESP_LOGI(TAG, "Capture #%u analysis dominant normalized buckets:", capture_no);
  bool used[RX_ANALYSIS_BIN_COUNT]{};
  for (uint8_t rank = 0; rank < 8; rank++) {
    uint16_t best_bin = 0;
    uint16_t best_count = 0;
    for (uint16_t bin = 0; bin < RX_ANALYSIS_BIN_COUNT; bin++) {
      if (!used[bin] && hist[bin] > best_count) {
        best_count = hist[bin];
        best_bin = bin;
      }
    }
    if (best_count == 0) {
      break;
    }
    used[best_bin] = true;
    ESP_LOGI(TAG, "  #%u: %u us count=%u", rank + 1, best_bin * RX_ANALYSIS_BIN_US, best_count);
  }

  // v0.9.0 analyzer: assign a compact symbol alphabet by ascending pulse bucket.
  static const char SYMBOLS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  char bin_symbol[RX_ANALYSIS_BIN_COUNT]{};
  uint8_t symbol_count = 0;
  for (uint16_t bin = 0; bin < RX_ANALYSIS_BIN_COUNT; bin++) {
    if (hist[bin] == 0) {
      continue;
    }
    if (symbol_count < sizeof(SYMBOLS) - 1) {
      bin_symbol[bin] = SYMBOLS[symbol_count++];
    } else {
      bin_symbol[bin] = '?';
    }
  }

  ESP_LOGI(TAG, "Capture #%u symbol alphabet, ascending pulse buckets:", capture_no);
  offset = snprintf(stream, sizeof(stream), "  alphabet:");
  on_line = 0;
  for (uint16_t bin = 0; bin < RX_ANALYSIS_BIN_COUNT; bin++) {
    if (hist[bin] == 0) {
      continue;
    }
    const int written = snprintf(stream + offset, sizeof(stream) - offset, " %c=%u", bin_symbol[bin], bin * RX_ANALYSIS_BIN_US);
    if (written <= 0 || written >= static_cast<int>(sizeof(stream) - offset) || ++on_line >= 12) {
      ESP_LOGI(TAG, "%s", stream);
      offset = snprintf(stream, sizeof(stream), "  alphabet:");
      on_line = 0;
      if (written > 0 && written < static_cast<int>(sizeof(stream) - offset)) {
        offset += snprintf(stream + offset, sizeof(stream) - offset, " %c=%u", bin_symbol[bin], bin * RX_ANALYSIS_BIN_US);
        on_line = 1;
      }
    } else {
      offset += written;
    }
  }
  if (on_line > 0) {
    ESP_LOGI(TAG, "%s", stream);
  }

  char symbols[RX_MAX_EDGES + 1]{};
  uint16_t symbol_len = 0;
  for (uint16_t i = 0; i < this->rx_edge_count_ && symbol_len < RX_MAX_EDGES; i++) {
    const uint16_t norm = this->rx_normalize_pulse_(this->rx_edges_[i]);
    if (norm == 0) {
      continue;
    }
    const uint16_t bin = norm / RX_ANALYSIS_BIN_US;
    if (bin < RX_ANALYSIS_BIN_COUNT && bin_symbol[bin] != 0) {
      symbols[symbol_len++] = bin_symbol[bin];
    }
  }
  symbols[symbol_len] = '\0';

  ESP_LOGI(TAG, "Capture #%u symbol stream, len=%u:", capture_no, symbol_len);
  for (uint16_t start = 0; start < symbol_len && start < RX_ANALYSIS_MAX_PRINTED_SYMBOLS; start += 64) {
    char line[96];
    uint16_t line_len = 0;
    for (uint16_t i = start; i < symbol_len && i < start + 64 && line_len < sizeof(line) - 1; i++) {
      line[line_len++] = symbols[i];
    }
    line[line_len] = '\0';
    ESP_LOGI(TAG, "  symbols[%03u-%03u]: %s", start, start + line_len - 1, line);
  }
  if (symbol_len > RX_ANALYSIS_MAX_PRINTED_SYMBOLS) {
    ESP_LOGI(TAG, "  symbols: ... truncated at %u of %u symbols", RX_ANALYSIS_MAX_PRINTED_SYMBOLS, symbol_len);
  }

  ESP_LOGI(TAG, "Capture #%u run-length stream:", capture_no);
  offset = snprintf(stream, sizeof(stream), "  rle:");
  uint16_t run_index = 0;
  uint16_t pos = 0;
  while (pos < symbol_len && run_index < 48) {
    const char sym = symbols[pos];
    uint16_t run = 1;
    while (pos + run < symbol_len && symbols[pos + run] == sym) {
      run++;
    }
    const int written = (run == 1)
                            ? snprintf(stream + offset, sizeof(stream) - offset, " %c", sym)
                            : snprintf(stream + offset, sizeof(stream) - offset, " %cx%u", sym, run);
    if (written <= 0 || written >= static_cast<int>(sizeof(stream) - offset)) {
      ESP_LOGI(TAG, "%s", stream);
      offset = snprintf(stream, sizeof(stream), "  rle:");
      continue;
    }
    offset += written;
    pos += run;
    run_index++;
  }
  if (offset > 6) {
    ESP_LOGI(TAG, "%s", stream);
  }
  if (pos < symbol_len) {
    ESP_LOGI(TAG, "  rle: ... truncated after %u runs of %u symbols", run_index, symbol_len);
  }

  // Simple motif scan: find the most repeated adjacent 2/3/4-symbol motif.
  uint8_t best_len = 0;
  uint16_t best_pos = 0;
  uint16_t best_repeats = 1;
  for (uint8_t motif_len = 2; motif_len <= 4; motif_len++) {
    for (uint16_t i = 0; i + motif_len * 2 <= symbol_len; i++) {
      uint16_t repeats = 1;
      bool still_matching = true;
      while (still_matching && i + (repeats + 1) * motif_len <= symbol_len) {
        for (uint8_t j = 0; j < motif_len; j++) {
          if (symbols[i + j] != symbols[i + repeats * motif_len + j]) {
            still_matching = false;
            break;
          }
        }
        if (still_matching) {
          repeats++;
        }
      }
      if (repeats > best_repeats) {
        best_repeats = repeats;
        best_len = motif_len;
        best_pos = i;
      }
    }
  }
  if (best_repeats > 1) {
    char motif[8]{};
    for (uint8_t i = 0; i < best_len && i < sizeof(motif) - 1; i++) {
      motif[i] = symbols[best_pos + i];
    }
    ESP_LOGI(TAG, "Capture #%u repeated motif: '%s' x%u at symbol offset %u", capture_no, motif, best_repeats, best_pos);
  } else {
    ESP_LOGI(TAG, "Capture #%u repeated motif: none detected", capture_no);
  }
}


bool RFBridgeComponent::rx_outprize_decode_from_index_(uint16_t start_index, OutprizeDecodeCandidate *candidate) const {
  *candidate = OutprizeDecodeCandidate{};
  candidate->start_index = start_index;
  candidate->stop_index = start_index;

  for (uint16_t i = start_index; i + 1 < this->rx_edge_count_ && candidate->bit_count < 64; i += 2) {
    const uint16_t pulse = this->rx_edges_[i];
    const uint16_t gap = this->rx_edges_[i + 1];

    // Outprize uses a short low pulse followed by a data-width high gap.
    // Be slightly more tolerant on pulse width because real captures sometimes
    // quantize a clean ~560us pulse as ~320/768us at the edges of the window.
    if (pulse < 300 || pulse > 850) {
      if (candidate->bit_count >= 24) {
        break;
      }
      candidate->invalid_count++;
      return false;
    }

    if (gap >= OUTPRIZE_SHORT_US_MIN && gap <= 850) {
      candidate->bits[candidate->bit_count++] = false;
    } else if (gap >= OUTPRIZE_LONG_US_MIN && gap <= OUTPRIZE_LONG_US_MAX) {
      candidate->bits[candidate->bit_count++] = true;
    } else {
      if (candidate->bit_count >= 24) {
        break;
      }
      candidate->invalid_count++;
      return false;
    }

    candidate->stop_index = i + 1;
  }

  if (candidate->bit_count < 24 || candidate->bit_count > 40) {
    return false;
  }

  uint32_t low24 = 0;
  const uint16_t low24_start = candidate->bit_count > 24 ? candidate->bit_count - 24 : 0;
  for (uint16_t i = low24_start; i < candidate->bit_count; i++) {
    low24 = (low24 << 1) | (candidate->bits[i] ? 1UL : 0UL);
  }
  candidate->low24 = low24 & 0xFFFFFF;

  uint64_t full_packet = 0;
  for (uint16_t i = 0; i < candidate->bit_count && i < 35; i++) {
    full_packet = (full_packet << 1) | (candidate->bits[i] ? 1ULL : 0ULL);
  }
  candidate->full_packet = full_packet;

  // The verified Outprize frame is 35 bits: 11-bit remote prefix + 24-bit command.
  // Shorter clipped decodes can still produce a valid Low24, but only a full 35-bit
  // decode should be trusted for learning/transmit identity.
  if (candidate->bit_count >= 35) {
    candidate->remote_id = static_cast<uint16_t>((full_packet >> 24) & 0x7FF);
  }

  candidate->score = this->rx_score_outprize_candidate_(candidate);
  candidate->valid = true;
  return true;
}

uint16_t RFBridgeComponent::rx_score_outprize_candidate_(OutprizeDecodeCandidate *candidate) const {
  uint16_t score = candidate->bit_count;
  const uint32_t low24 = candidate->low24;

  // All verified Outprize packets in this project have the fixed 0x60 high byte.
  if ((low24 & 0xFF0000UL) == 0x600000UL) {
    score += 1000;
  }

  // Known vent command nibble values: idle/off, close, open, stop.
  const uint8_t vent = static_cast<uint8_t>(low24 & 0x0F);
  if (vent == 0x00 || vent == 0x04 || vent == 0x08 || vent == 0x0C) {
    score += 100;
  }

  // Speed/rain/direction field should be one of the documented Gray-code states.
  const uint8_t state_nibble = static_cast<uint8_t>((low24 >> 4) & 0x0F);
  switch (state_nibble) {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
      score += 20;
      break;
    default:
      break;
  }

  if (candidate->invalid_count > 0) {
    score = candidate->invalid_count >= score ? 0 : score - candidate->invalid_count;
  }

  return score;
}

bool RFBridgeComponent::rx_log_outprize_decode_(uint32_t capture_no) {
  this->rx_last_outprize_like_ = false;

  if (this->rx_edge_count_ < OUTPRIZE_MIN_EDGES) {
    if (this->diagnostic_logging_) {
      ESP_LOGD(TAG, "Capture #%u Outprize learner: skipped (edge count %u below %u)",
               capture_no, this->rx_edge_count_, OUTPRIZE_MIN_EDGES);
    }
    return false;
  }

  uint16_t shortish = 0;
  uint16_t longish = 0;
  uint16_t syncish = 0;
  for (uint16_t i = 0; i < this->rx_edge_count_; i++) {
    const uint16_t t = this->rx_edges_[i];
    if (t >= OUTPRIZE_SHORT_US_MIN && t <= OUTPRIZE_SHORT_US_MAX) {
      shortish++;
    } else if (t >= OUTPRIZE_LONG_US_MIN && t <= OUTPRIZE_LONG_US_MAX) {
      longish++;
    } else if (t >= OUTPRIZE_SYNC_US_MIN && t <= OUTPRIZE_SYNC_US_MAX) {
      syncish++;
    }
  }

  if (shortish < 18 || longish < 4) {
    if (this->diagnostic_logging_) {
      ESP_LOGD(TAG, "Capture #%u Outprize learner: skipped (short=%u long=%u sync=%u)",
               capture_no, shortish, longish, syncish);
    }
    return false;
  }

  this->rx_last_outprize_like_ = true;

  OutprizeDecodeCandidate best{};
  uint16_t valid_candidates = 0;

  // v1.3.16: keep the top candidate alignments for Power Off diagnostics.
  // Power Off has repeatedly appeared as clipped/shifted 30/34-bit candidates,
  // while Fan Awake usually lands as a clean 35-bit frame.  The top list lets us
  // see every plausible alignment instead of only the final winner.
  static constexpr uint8_t OUTPRIZE_DIAG_TOP_CANDIDATES = 16;
  OutprizeDecodeCandidate top_candidates[OUTPRIZE_DIAG_TOP_CANDIDATES]{};
  uint8_t top_candidate_count = 0;
  auto add_top_candidate = [&](const OutprizeDecodeCandidate &cand) {
    if (!cand.valid) {
      return;
    }
    uint8_t pos = top_candidate_count;
    if (top_candidate_count < OUTPRIZE_DIAG_TOP_CANDIDATES) {
      top_candidates[top_candidate_count++] = cand;
    } else {
      // If the new candidate is not better than the current last entry, drop it.
      const auto &last = top_candidates[OUTPRIZE_DIAG_TOP_CANDIDATES - 1];
      if (cand.score < last.score || (cand.score == last.score && cand.bit_count <= last.bit_count)) {
        return;
      }
      top_candidates[OUTPRIZE_DIAG_TOP_CANDIDATES - 1] = cand;
      pos = OUTPRIZE_DIAG_TOP_CANDIDATES - 1;
    }

    // Insertion sort by score, then exact 35-bit prefix match, then bit count.
    while (pos > 0) {
      const auto &prev = top_candidates[pos - 1];
      const auto &cur = top_candidates[pos];
      const bool cur_exact = cur.bit_count == OUTPRIZE_TX_BITS && cur.remote_id == OUTPRIZE_DEFAULT_PREFIX;
      const bool prev_exact = prev.bit_count == OUTPRIZE_TX_BITS && prev.remote_id == OUTPRIZE_DEFAULT_PREFIX;
      bool better = false;
      if (cur.score > prev.score) {
        better = true;
      } else if (cur.score == prev.score && cur_exact && !prev_exact) {
        better = true;
      } else if (cur.score == prev.score && cur_exact == prev_exact && cur.bit_count > prev.bit_count) {
        better = true;
      }
      if (!better) {
        break;
      }
      auto tmp = top_candidates[pos - 1];
      top_candidates[pos - 1] = top_candidates[pos];
      top_candidates[pos] = tmp;
      pos--;
    }
  };

  // Scan every plausible pulse alignment and use the score to prefer the
  // verified 0x60xxxx packet family over a merely-longer but shifted decode.
  for (uint16_t i = 0; i + 1 < this->rx_edge_count_; i++) {
    if (this->rx_edges_[i] < 300 || this->rx_edges_[i] > 850) {
      continue;
    }

    OutprizeDecodeCandidate trial{};
    if (!this->rx_outprize_decode_from_index_(i, &trial)) {
      continue;
    }

    valid_candidates++;
    add_top_candidate(trial);
    if (!best.valid || trial.score > best.score ||
        (trial.score == best.score && trial.bit_count > best.bit_count)) {
      best = trial;
    }
  }

  if (!best.valid) {
    ESP_LOGI(TAG, "Capture #%u Outprize decoder: candidate_no_decode edges=%u short=%u long=%u sync=%u",
             capture_no, this->rx_edge_count_, shortish, longish, syncish);
    return false;
  }

  char binary[72]{};
  for (uint16_t i = 0; i < best.bit_count && i < sizeof(binary) - 1; i++) {
    binary[i] = best.bits[i] ? '1' : '0';
  }

  char hexbuf[48]{};
  uint16_t hpos = 0;
  uint8_t nibble = 0;
  uint8_t used = 0;
  for (uint16_t i = 0; i < best.bit_count && hpos + 3 < sizeof(hexbuf); i++) {
    nibble = static_cast<uint8_t>((nibble << 1) | (best.bits[i] ? 1 : 0));
    used++;
    if (used == 4) {
      hpos += snprintf(hexbuf + hpos, sizeof(hexbuf) - hpos, "%X ", nibble);
      nibble = 0;
      used = 0;
    }
  }
  if (used > 0 && hpos + 2 < sizeof(hexbuf)) {
    nibble <<= (4 - used);
    hpos += snprintf(hexbuf + hpos, sizeof(hexbuf) - hpos, "%X", nibble);
  } else if (hpos > 0 && hexbuf[hpos - 1] == ' ') {
    hexbuf[hpos - 1] = '\0';
  }

  const char *confidence = "low";
  if (best.score >= 1130) {
    confidence = "excellent";
  } else if (best.score >= 1100) {
    confidence = "good";
  } else if (best.score >= 1050) {
    confidence = "fair";
  }

  if (best.valid && best.bit_count >= OUTPRIZE_TX_BITS && this->sequence_active_) {
    this->outprize_learned_valid_ = true;
    this->outprize_learned_full35_ = best.full_packet & 0x7FFFFFFFFULL;
    this->outprize_learned_low24_ = best.low24 & 0xFFFFFFUL;
    this->outprize_learned_remote_id_ = best.remote_id & 0x7FF;
    this->outprize_learned_bits_ = best.bit_count;
    this->outprize_learned_capture_no_ = capture_no;
    this->outprize_learned_score_ = best.score;
    this->outprize_learned_start_index_ = best.start_index;
    this->outprize_learned_stop_index_ = best.stop_index;
    this->outprize_learned_edge_count_ = this->rx_edge_count_;
    this->outprize_learned_initial_level_ = this->rx_capture_initial_level_;
    for (uint16_t copy_i = 0; copy_i < this->rx_edge_count_ && copy_i < RX_MAX_EDGES; copy_i++) {
      this->outprize_learned_edges_[copy_i] = this->rx_edges_[copy_i];
      this->outprize_learned_levels_[copy_i] = this->rx_levels_[copy_i];
    }
    strncpy(this->outprize_learned_binary_, binary, sizeof(this->outprize_learned_binary_) - 1);
    this->outprize_learned_binary_[sizeof(this->outprize_learned_binary_) - 1] = '\0';
    ESP_LOGI(TAG, "OUTPRIZE_LEARNED capture=%u full35=0x%09llX remote=0x%03X low24=0x%06X bits=%u score=%u stream=%s",
             capture_no, static_cast<unsigned long long>(this->outprize_learned_full35_),
             this->outprize_learned_remote_id_, this->outprize_learned_low24_,
             this->outprize_learned_bits_, this->outprize_learned_score_, this->outprize_learned_binary_);
  }

  if (best.valid && best.bit_count >= OUTPRIZE_TX_BITS && millis() >= this->suppress_oem_until_ms_) {
    const uint32_t low24 = best.low24 & 0xFFFFFFUL;
    const bool powered = low24 != 0x600000UL;
    const uint8_t speed_percent = this->decode_outprize_speed_(low24);
    const bool direction_in = (low24 & 0x20) != 0;
    const bool rain_enabled = (low24 & 0x10) != 0;
    const uint8_t vent_command = static_cast<uint8_t>(low24 & 0x0C);

    // Emit every complete valid frame for discovery and multi-remote routing.
    this->outprize_frame_callback_.call(best.remote_id & 0x7FF, low24, powered, speed_percent,
                                        direction_in, rain_enabled, vent_command,
                                        this->rx_last_trigger_rssi_dbm_);

    // Preserve the legacy single-remote diagnostic cache until the HA integration
    // replaces the temporary ESPHome sensors.
    if (best.remote_id == (this->outprize_remote_id_ & 0x7FF)) {
      this->update_outprize_state_from_low24_(low24, OutprizeCommandSource::OEM_REMOTE);
    }
  }

  if (!this->diagnostic_logging_) {
    if (best.bit_count >= 35) {
      ESP_LOGI(TAG, "OUTPRIZE remote=0x%03X full35=0x%09llX low24=0x%06X confidence=%s score=%u candidates=%u edges=%u rssi=%d dBm",
               best.remote_id, static_cast<unsigned long long>(best.full_packet), best.low24, confidence, best.score, valid_candidates,
               this->rx_edge_count_, this->rx_last_trigger_rssi_dbm_);
    } else {
      ESP_LOGI(TAG, "OUTPRIZE remote=? full35=? low24=0x%06X confidence=%s score=%u candidates=%u edges=%u bits=%u rssi=%d dBm",
               best.low24, confidence, best.score, valid_candidates, this->rx_edge_count_, best.bit_count,
               this->rx_last_trigger_rssi_dbm_);
    }
    return true;
  }

  ESP_LOGI(TAG, "===== OUTPRIZE_PACKET =====");
  ESP_LOGI(TAG, "Decoder: PWM gap short=0 long=1");
  ESP_LOGI(TAG, "Capture: #%u RSSI trigger=%d dBm end=%d dBm", capture_no, this->rx_last_trigger_rssi_dbm_, this->rx_last_rssi_dbm_);
  ESP_LOGI(TAG, "Edges: %u  DecodeStartIndex: %u  StopIndex: %u  Bits: %u  Candidates: %u  Score: %u  Confidence: %s",
           this->rx_edge_count_, best.start_index, best.stop_index, best.bit_count, valid_candidates, best.score, confidence);
  ESP_LOGI(TAG, "Binary: %s", binary);
  ESP_LOGI(TAG, "Hex: %s", hexbuf);
  ESP_LOGI(TAG, "Remote ID / prefix: %s0x%03X", best.bit_count >= 35 ? "" : "? ", best.remote_id);
  ESP_LOGI(TAG, "Full35: 0x%09llX", static_cast<unsigned long long>(best.full_packet));
  ESP_LOGI(TAG, "Low24: 0x%06X", best.low24);
  ESP_LOGI(TAG, "Timing counts: short=%u long=%u sync=%u", shortish, longish, syncish);

  // v1.3.16 Power Off alignment diagnostics.  Print the top plausible starts,
  // including clipped 30-34 bit decodes, so we can see whether Power Off is a
  // real short frame or simply a frame-start synchronization failure.
  ESP_LOGI(TAG, "===== OUTPRIZE_CANDIDATES_30_TO_35 =====");
  uint8_t printed = 0;
  for (uint8_t ci = 0; ci < top_candidate_count; ci++) {
    const auto &cand = top_candidates[ci];
    if (cand.bit_count < 30 || cand.bit_count > 35) {
      continue;
    }
    char cbinary[72]{};
    for (uint16_t bi = 0; bi < cand.bit_count && bi < sizeof(cbinary) - 1; bi++) {
      cbinary[bi] = cand.bits[bi] ? '1' : '0';
    }
    const bool exact_prefix = cand.bit_count == OUTPRIZE_TX_BITS && cand.remote_id == OUTPRIZE_DEFAULT_PREFIX;
    const bool power_family = (cand.low24 & 0xFFFFC0UL) == 0x600000UL;
    ESP_LOGI(TAG, "Candidate[%u] start=%u stop=%u bits=%u score=%u exact_prefix=%s power_family=%s remote=%s0x%03X full=0x%09llX low24=0x%06X stream=%s",
             printed + 1, cand.start_index, cand.stop_index, cand.bit_count, cand.score,
             YESNO(exact_prefix), YESNO(power_family), cand.bit_count >= OUTPRIZE_TX_BITS ? "" : "? ",
             cand.remote_id, static_cast<unsigned long long>(cand.full_packet), cand.low24, cbinary);
    printed++;
  }
  if (printed == 0) {
    ESP_LOGI(TAG, "No 30-35 bit candidates in top candidate list");
  }

  if ((best.low24 & 0xFFFFFFUL) == 0x600000UL && best.bit_count != OUTPRIZE_TX_BITS) {
    ESP_LOGW(TAG, "POWER_OFF_DIAG: best low24 looks like Power Off but bit_count=%u, start=%u; likely clipped/misaligned frame",
             best.bit_count, best.start_index);
  } else if ((best.low24 & 0xFFFFFFUL) == 0x600040UL && best.bit_count == OUTPRIZE_TX_BITS) {
    ESP_LOGI(TAG, "POWER_OFF_DIAG: clean Fan Awake frame learned; compare against Power Off candidate list above");
  }

  ESP_LOGI(TAG, "====================================");
  return true;
}

void RFBridgeComponent::rx_reset_packet_(uint32_t now_us, bool level) {
  this->rx_edge_count_ = 0;
  this->rx_packet_start_us_ = now_us;
  this->rx_last_edge_us_ = now_us;
  this->rx_last_level_ = level;
}

uint32_t RFBridgeComponent::outprize_speed_base_(uint8_t speed_percent) const {
  // Verified Outprize speed table. Values are the speed portion of Low24,
  // excluding direction/rain/vent modifiers. 0% is the awake/fan-off command.
  //
  //  0%: 0x040  (POWER ON / FAN OFF / awake idle)
  // 10%: 0x440
  // 20%: 0x240
  // 30%: 0x640
  // 40%: 0x140
  // 50%: 0x540
  // 60%: 0x340
  // 70%: 0x740
  // 80%: 0x0C0
  // 90%: 0x4C0
  //100%: 0x2C0
  switch (speed_percent) {
    case 0: return 0x040;
    case 10: return 0x440;
    case 20: return 0x240;
    case 30: return 0x640;
    case 40: return 0x140;
    case 50: return 0x540;
    case 60: return 0x340;
    case 70: return 0x740;
    case 80: return 0x0C0;
    case 90: return 0x4C0;
    case 100: return 0x2C0;
    default:
      return this->outprize_speed_base_(static_cast<uint8_t>(((speed_percent + 5) / 10) * 10));
  }
}

uint32_t RFBridgeComponent::encode_outprize_low24(uint8_t speed_percent, OutprizeDirection direction,
                                                  bool rain_enabled, OutprizeVentCommand vent_command) const {
  return this->outprize_codec_.encode_low24(speed_percent, direction, rain_enabled, vent_command);
}

bool RFBridgeComponent::send_outprize(uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                                      OutprizeVentCommand vent_command) {
  return this->send_outprize(this->outprize_remote_id_, speed_percent, direction, rain_enabled, vent_command);
}

bool RFBridgeComponent::send_outprize(uint32_t remote_id, uint8_t speed_percent, OutprizeDirection direction,
                                      bool rain_enabled, OutprizeVentCommand vent_command) {
  const uint32_t low24 = this->encode_outprize_low24(speed_percent, direction, rain_enabled, vent_command);
  ESP_LOGI(TAG, "OUTPRIZE TX request remote_prefix=0x%03X speed=%u direction=%s rain=%s vent=0x%02X low24=0x%06X",
           remote_id & 0x7FF, speed_percent, direction == OutprizeDirection::IN ? "IN" : "OUT",
           YESNO(rain_enabled), static_cast<uint8_t>(vent_command), low24 & 0xFFFFFF);
  return this->send_outprize_low24(remote_id, low24, 3);
}

bool RFBridgeComponent::send_outprize_low24(uint32_t low24, uint8_t repeats) {
  return this->send_outprize_low24(this->outprize_remote_id_, low24, repeats);
}

bool RFBridgeComponent::send_outprize_low24(uint32_t remote_id, uint32_t low24, uint8_t repeats) {
  ESP_LOGI(TAG, "OUTPRIZE TX raw remote=0x%03X low24=0x%06X full35=0x%09llX repeats=%u", remote_id & 0x7FF,
           low24 & 0xFFFFFF, static_cast<unsigned long long>(((static_cast<uint64_t>(remote_id & 0x7FF)) << 24) | (low24 & 0xFFFFFFULL)), repeats);
  return this->transmit_low24_(remote_id, low24, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_lsb(uint8_t repeats) {
  ESP_LOGI(TAG, "OUTPRIZE TX test helper: Power Off LSB_NORMAL");
  return this->send_outprize_low24_lsb(0x600000, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_inv(uint8_t repeats) {
  ESP_LOGI(TAG, "OUTPRIZE TX test helper: Power Off MSB_INVERTED");
  return this->send_outprize_low24_inverted(0x600000, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_inv_lsb(uint8_t repeats) {
  ESP_LOGI(TAG, "OUTPRIZE TX test helper: Power Off LSB_INVERTED");
  return this->send_outprize_low24_lsb_inverted(0x600000, repeats);
}

bool RFBridgeComponent::send_outprize_raw_oem_power_off(uint8_t repeats) {
  // Current best-known OEM Power Off frame from the Outprize decoder:
  // 11-bit prefix 0x6CF + 24-bit low command 0x600000. This helper is
  // intentionally explicit so the YAML test button calls a method that exists.
  constexpr uint64_t OEM_POWER_OFF_FULL35 = ((static_cast<uint64_t>(OUTPRIZE_DEFAULT_PREFIX) << 24) | 0x600000ULL);
  ESP_LOGI(TAG, "OUTPRIZE TX test helper: Raw OEM Power Off full35=0x%09llX",
           static_cast<unsigned long long>(OEM_POWER_OFF_FULL35));
  return this->send_outprize_raw_full35(OEM_POWER_OFF_FULL35, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_433900(uint8_t repeats) {
  return this->transmit_power_off_with_frequency_(0x10, 0xB0, 0x3F, "433.900 MHz", repeats);
}

bool RFBridgeComponent::send_outprize_power_off_433920(uint8_t repeats) {
  return this->transmit_power_off_with_frequency_(0x10, 0xB0, 0x71, "433.920 MHz", repeats);
}

bool RFBridgeComponent::send_outprize_power_off_433940(uint8_t repeats) {
  return this->transmit_power_off_with_frequency_(0x10, 0xB0, 0xA4, "433.940 MHz", repeats);
}

bool RFBridgeComponent::send_outprize_power_off_433950(uint8_t repeats) {
  return this->transmit_power_off_with_frequency_(0x10, 0xB0, 0xBD, "433.950 MHz", repeats);
}

bool RFBridgeComponent::send_outprize_power_off_433970(uint8_t repeats) {
  return this->transmit_power_off_with_frequency_(0x10, 0xB0, 0xEF, "433.970 MHz", repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_default(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_DEFAULT", false, 0xC0, 0x11, 0x30, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_inverted_ook(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_INVERTED_OOK", true, 0xC0, 0x11, 0x30, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_pa_80(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_PA_0x80", false, 0x80, 0x11, 0x30, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_pa_60(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_PA_0x60", false, 0x60, 0x11, 0x30, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_frend0_10(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_FREND0_0x10", false, 0xC0, 0x10, 0x30, repeats);
}

bool RFBridgeComponent::send_outprize_power_off_rf_mdmcfg2_33(uint8_t repeats) {
  return this->transmit_power_off_with_rf_profile_("RF_MDMCFG2_0x33", false, 0xC0, 0x11, 0x33, repeats);
}




bool RFBridgeComponent::send_outprize_power_off_soft_ask_profile(uint8_t profile, uint8_t repeats) {
  switch (profile) {
    case 1:
      // Softer baseline: lower PA and alternate front-end, non-inverted.
      return this->transmit_power_off_with_rf_profile_("SOFT_ASK_1_PA80_FREND10", false, 0x80, 0x10, 0x30, repeats);
    case 2:
      // Lower PA plus alternate MDMCFG2 used as a wider/alternate ASK-style modulation profile.
      return this->transmit_power_off_with_rf_profile_("SOFT_ASK_2_PA60_FREND10_MDMCFG33", false, 0x60, 0x10, 0x33, repeats);
    case 3:
      // Full PA but alternate front-end/modulation profile.
      return this->transmit_power_off_with_rf_profile_("SOFT_ASK_3_PA_C0_FREND10_MDMCFG33", false, 0xC0, 0x10, 0x33, repeats);
    case 4:
      // Soft profile with inverted electrical OOK polarity as a final envelope/polarity check.
      return this->transmit_power_off_with_rf_profile_("SOFT_ASK_4_INVERTED_PA80", true, 0x80, 0x10, 0x30, repeats);
    default:
      ESP_LOGW(TAG, "OUTPRIZE TX soft ASK helper: unknown profile %u; valid profiles are 1..4", profile);
      return false;
  }
}
bool RFBridgeComponent::send_outprize_power_off_profile(uint8_t profile, uint8_t repeats) {
  switch (profile) {
    case 1:
      return this->send_outprize_power_off_rf_default(repeats);
    case 2:
      return this->send_outprize_power_off_rf_inverted_ook(repeats);
    case 3:
      return this->send_outprize_power_off_rf_pa_80(repeats);
    case 4:
      return this->send_outprize_power_off_rf_pa_60(repeats);
    case 5:
      return this->send_outprize_power_off_rf_frend0_10(repeats);
    case 6:
      return this->send_outprize_power_off_rf_mdmcfg2_33(repeats);
    default:
      ESP_LOGW(TAG, "OUTPRIZE TX RF profile helper: unknown profile %u; valid profiles are 1..6", profile);
      return false;
  }
}

void RFBridgeComponent::cc1101_set_tx_frequency_(uint8_t freq2, uint8_t freq1, uint8_t freq0, const char *label) {
  this->tx_freq2_ = freq2;
  this->tx_freq1_ = freq1;
  this->tx_freq0_ = freq0;
  this->tx_freq_label_ = label;
}

bool RFBridgeComponent::transmit_power_off_with_frequency_(uint8_t freq2, uint8_t freq1, uint8_t freq0, const char *label, uint8_t repeats) {
  const uint8_t old2 = this->tx_freq2_;
  const uint8_t old1 = this->tx_freq1_;
  const uint8_t old0 = this->tx_freq0_;
  const char *old_label = this->tx_freq_label_;
  ESP_LOGI(TAG, "OUTPRIZE TX frequency trim helper: Power Off at %s FREQ=0x%02X%02X%02X", label, freq2, freq1, freq0);
  this->cc1101_set_tx_frequency_(freq2, freq1, freq0, label);
  const bool ok = this->send_outprize_low24(0x600000, repeats);
  this->cc1101_set_tx_frequency_(old2, old1, old0, old_label);
  return ok;
}

bool RFBridgeComponent::transmit_power_off_with_rf_profile_(const char *label, bool inverted_ook, uint8_t pa, uint8_t frend0, uint8_t mdmcfg2, uint8_t repeats) {
  const bool old_inverted = this->tx_ook_inverted_;
  const uint8_t old_pa = this->tx_pa_value_;
  const uint8_t old_frend0 = this->tx_frend0_value_;
  const uint8_t old_mdmcfg2 = this->tx_mdmcfg2_value_;

  ESP_LOGI(TAG, "OUTPRIZE TX RF profile helper: %s inverted_ook=%s PA=0x%02X FREND0=0x%02X MDMCFG2=0x%02X",
           label, YESNO(inverted_ook), pa, frend0, mdmcfg2);
  this->tx_ook_inverted_ = inverted_ook;
  this->tx_pa_value_ = pa;
  this->tx_frend0_value_ = frend0;
  this->tx_mdmcfg2_value_ = mdmcfg2;
  const bool ok = this->transmit_low24_mode_(this->outprize_remote_id_, 0x600000, repeats, TxFrameMode::MSB_NORMAL, label);
  this->tx_ook_inverted_ = old_inverted;
  this->tx_pa_value_ = old_pa;
  this->tx_frend0_value_ = old_frend0;
  this->tx_mdmcfg2_value_ = old_mdmcfg2;
  return ok;
}

bool RFBridgeComponent::send_outprize_low24_lsb(uint32_t low24, uint8_t repeats) {
  return this->transmit_low24_mode_(this->outprize_remote_id_, low24, repeats, TxFrameMode::LSB_NORMAL, "LOW24_LSB_NORMAL");
}

bool RFBridgeComponent::send_outprize_low24_inverted(uint32_t low24, uint8_t repeats) {
  return this->transmit_low24_mode_(this->outprize_remote_id_, low24, repeats, TxFrameMode::MSB_INVERTED, "LOW24_MSB_INVERTED");
}

bool RFBridgeComponent::send_outprize_low24_lsb_inverted(uint32_t low24, uint8_t repeats) {
  return this->transmit_low24_mode_(this->outprize_remote_id_, low24, repeats, TxFrameMode::LSB_INVERTED, "LOW24_LSB_INVERTED");
}

bool RFBridgeComponent::send_outprize_raw_full35(uint64_t full35, uint8_t repeats) {
  return this->transmit_full35_mode_(full35, repeats, TxFrameMode::MSB_NORMAL, "RAW_FULL35_MSB_NORMAL");
}

bool RFBridgeComponent::send_outprize_raw_full35_lsb(uint64_t full35, uint8_t repeats) {
  return this->transmit_full35_mode_(full35, repeats, TxFrameMode::LSB_NORMAL, "RAW_FULL35_LSB_NORMAL");
}

void RFBridgeComponent::cc1101_configure_ook_async_tx_() {
  this->cc1101_enter_idle_();
  this->cc1101_strobe_(cc1101::SFTX);

  // v1.3.5+ deliberately programs a full async OOK TX profile instead of
  // inheriting the RX/sniffer profile.  The RX decoder is restored after each
  // transmission by cc1101_configure_ook_async_rx_().
  ESP_LOGI(TAG, "Configuring CC1101 full async OOK TX profile at %s FREQ=0x%02X%02X%02X (PA=0x%02X FREND0=0x%02X MDMCFG2=0x%02X inverted_ook=%s)", this->tx_freq_label_, this->tx_freq2_, this->tx_freq1_, this->tx_freq0_, this->tx_pa_value_, this->tx_frend0_value_, this->tx_mdmcfg2_value_, YESNO(this->tx_ook_inverted_));

  this->cc1101_write_reg_(cc1101::IOCFG2, cc1101::GDO_IOCFG2_KNOWN_GOOD);
  this->cc1101_write_reg_(cc1101::IOCFG1, 0x2E);
  this->cc1101_write_reg_(cc1101::IOCFG0, cc1101::GDO_SERIAL_DATA);
  this->cc1101_write_reg_(cc1101::FIFOTHR, 0x47);
  this->cc1101_write_reg_(cc1101::SYNC1, 0xD3);
  this->cc1101_write_reg_(cc1101::SYNC0, 0x91);
  this->cc1101_write_reg_(cc1101::PKTLEN, 0xFF);
  this->cc1101_write_reg_(cc1101::PKTCTRL1, 0x04);
  this->cc1101_write_reg_(cc1101::PKTCTRL0, cc1101::PKTCTRL0_ASYNC_INFINITE);
  this->cc1101_write_reg_(cc1101::ADDR, 0x00);
  this->cc1101_write_reg_(cc1101::CHANNR, 0x00);
  this->cc1101_write_reg_(cc1101::FSCTRL1, 0x06);
  this->cc1101_write_reg_(cc1101::FSCTRL0, 0x00);
  this->cc1101_write_reg_(cc1101::FREQ2, this->tx_freq2_);
  this->cc1101_write_reg_(cc1101::FREQ1, this->tx_freq1_);
  this->cc1101_write_reg_(cc1101::FREQ0, this->tx_freq0_);
  this->cc1101_write_reg_(cc1101::MDMCFG4, 0xF6);
  this->cc1101_write_reg_(cc1101::MDMCFG3, 0x43);
  this->cc1101_write_reg_(cc1101::MDMCFG2, this->tx_mdmcfg2_value_);
  this->cc1101_write_reg_(cc1101::MDMCFG1, 0x22);
  this->cc1101_write_reg_(cc1101::MDMCFG0, 0xF8);
  this->cc1101_write_reg_(cc1101::DEVIATN, 0x00);
  this->cc1101_write_reg_(cc1101::MCSM2, 0x07);
  this->cc1101_write_reg_(cc1101::MCSM1, 0x30);
  this->cc1101_write_reg_(cc1101::MCSM0, 0x18);
  this->cc1101_write_reg_(cc1101::FOCCFG, 0x16);
  this->cc1101_write_reg_(cc1101::BSCFG, 0x6C);
  this->cc1101_write_reg_(cc1101::AGCCTRL2, 0x04);
  this->cc1101_write_reg_(cc1101::AGCCTRL1, 0x00);
  this->cc1101_write_reg_(cc1101::AGCCTRL0, 0x91);
  this->cc1101_write_reg_(cc1101::FREND1, 0x56);
  this->cc1101_write_reg_(cc1101::FREND0, this->tx_frend0_value_);
  this->cc1101_write_reg_(cc1101::FSCAL3, 0xE9);
  this->cc1101_write_reg_(cc1101::FSCAL2, 0x2A);
  this->cc1101_write_reg_(cc1101::FSCAL1, 0x00);
  this->cc1101_write_reg_(cc1101::FSCAL0, 0x1F);
  this->cc1101_write_reg_(cc1101::TEST2, 0x81);
  this->cc1101_write_reg_(cc1101::TEST1, 0x35);
  this->cc1101_write_reg_(cc1101::TEST0, 0x09);
  this->cc1101_write_patable_(this->tx_pa_value_);

  ESP_LOGI(TAG, "  IOCFG0   = 0x%02X", this->cc1101_read_reg_(cc1101::IOCFG0));
  ESP_LOGI(TAG, "  PKTCTRL0 = 0x%02X", this->cc1101_read_reg_(cc1101::PKTCTRL0));
  ESP_LOGI(TAG, "  FREND0   = 0x%02X", this->cc1101_read_reg_(cc1101::FREND0));
  ESP_LOGI(TAG, "  MDMCFG2  = 0x%02X", this->cc1101_read_reg_(cc1101::MDMCFG2));
  ESP_LOGI(TAG, "  FREQ     = 0x%02X%02X%02X (%s)", this->cc1101_read_reg_(cc1101::FREQ2), this->cc1101_read_reg_(cc1101::FREQ1), this->cc1101_read_reg_(cc1101::FREQ0), this->tx_freq_label_);
  ESP_LOGI(TAG, "  PATABLE  = 0x%02X programmed", this->tx_pa_value_);
}

bool RFBridgeComponent::cc1101_calibrate_for_tx_() {
  ESP_LOGI(TAG, "CC1101 TX calibration: SCAL");
  const uint8_t scal_status = this->cc1101_strobe_(cc1101::SCAL);
  ESP_LOGI(TAG, "CC1101 TX calibration SCAL status=0x%02X", scal_status);

  bool idle_seen = false;
  uint8_t marc = 0xFF;
  for (uint8_t i = 0; i < 40; i++) {
    delayMicroseconds(250);
    marc = this->cc1101_read_status_(cc1101::MARCSTATE) & 0x1F;
    if (marc == 0x01) {
      idle_seen = true;
      break;
    }
  }

  ESP_LOGI(TAG, "CC1101 TX calibration result idle_seen=%s MARCSTATE=0x%02X FSCAL1=0x%02X FSCAL0=0x%02X",
           YESNO(idle_seen), marc, this->cc1101_read_reg_(cc1101::FSCAL1), this->cc1101_read_reg_(cc1101::FSCAL0));
  return idle_seen;
}

void RFBridgeComponent::tx_write_data_(bool level) {
  if (this->gdo0_pin_ != nullptr) {
    this->gdo0_pin_->digital_write(level);
  }
}

void RFBridgeComponent::tx_write_carrier_(bool on) {
  // Normal async OOK profile observed so far: ESP LOW = carrier on, ESP HIGH = carrier off.
  // v1.3.18 can invert this electrical polarity without changing payload bits.
  const bool level = this->tx_ook_inverted_ ? on : !on;
  this->tx_write_data_(level);
}

void RFBridgeComponent::tx_log_marcstate_(const char *stage) {
  const uint8_t marc = this->cc1101_read_status_(cc1101::MARCSTATE);
  ESP_LOGI(TAG, "CC1101 TX state %-18s MARCSTATE=0x%02X", stage, marc);
}

void RFBridgeComponent::tx_dump_status_(const char *stage) {
  ESP_LOGI(TAG, "CC1101 TX status %-18s MARCSTATE=0x%02X PKTSTATUS=0x%02X TXBYTES=0x%02X FREQEST=0x%02X RSSI=%d dBm",
           stage,
           this->cc1101_read_status_(cc1101::MARCSTATE),
           this->cc1101_read_status_(cc1101::PKTSTATUS),
           this->cc1101_read_status_(cc1101::TXBYTES),
           this->cc1101_read_status_(cc1101::FREQEST),
           this->cc1101_read_rssi_dbm_());
}

void RFBridgeComponent::tx_log_frame_bits_(uint64_t frame, uint8_t bits, TxFrameMode mode, const char *label) const {
  char bit_string[72];
  if (bits >= sizeof(bit_string)) bits = sizeof(bit_string) - 1;
  for (uint8_t i = 0; i < bits; i++) {
    const uint8_t source_bit = (mode == TxFrameMode::LSB_NORMAL || mode == TxFrameMode::LSB_INVERTED) ? i : (bits - 1 - i);
    bool bit_value = ((frame >> source_bit) & 0x01ULL) != 0;
    if (mode == TxFrameMode::MSB_INVERTED || mode == TxFrameMode::LSB_INVERTED) {
      bit_value = !bit_value;
    }
    bit_string[i] = bit_value ? '1' : '0';
  }
  bit_string[bits] = '\0';
  ESP_LOGI(TAG, "OUTPRIZE TX bits label=%s mode=%u bits=%u full=0x%09llX stream=%s", label, static_cast<unsigned>(mode), bits,
           static_cast<unsigned long long>(frame), bit_string);
}

void RFBridgeComponent::tx_send_frame_bits_(uint64_t frame, uint8_t bits, TxFrameMode mode) {
  // v1.3.9 keeps the v1.3.8 RF envelope/timing and varies only logical bit order/inversion.
  // OOK envelope:
  //   ESP LOW  = carrier on pulse
  //   ESP HIGH = carrier off gap
  for (uint8_t i = 0; i < bits; i++) {
    const uint8_t source_bit = (mode == TxFrameMode::LSB_NORMAL || mode == TxFrameMode::LSB_INVERTED) ? i : (bits - 1 - i);
    bool one = ((frame >> source_bit) & 0x01ULL) != 0;
    if (mode == TxFrameMode::MSB_INVERTED || mode == TxFrameMode::LSB_INVERTED) {
      one = !one;
    }
    this->tx_write_carrier_(true);  // carrier on
    delayMicroseconds(OUTPRIZE_TX_PULSE_US);
    this->tx_write_carrier_(false); // carrier off / symbol gap
    delayMicroseconds(one ? OUTPRIZE_TX_ONE_GAP_US : OUTPRIZE_TX_ZERO_GAP_US);
  }

  this->tx_write_carrier_(false);
}

void RFBridgeComponent::tx_send_outprize_frame_(uint32_t prefix, uint32_t low24) {
  const uint64_t frame = ((static_cast<uint64_t>(prefix & 0x7FF)) << 24) | (low24 & 0xFFFFFFULL);
  this->tx_send_frame_bits_(frame, OUTPRIZE_TX_BITS, TxFrameMode::MSB_NORMAL);
}


bool RFBridgeComponent::transmit_low24_(uint32_t remote_id, uint32_t low24, uint8_t repeats) {
  return this->transmit_low24_mode_(remote_id, low24, repeats, TxFrameMode::MSB_NORMAL, "LOW24_MSB_NORMAL");
}

bool RFBridgeComponent::transmit_low24_mode_(uint32_t remote_id, uint32_t low24, uint8_t repeats, TxFrameMode mode, const char *label) {
  const uint32_t prefix = remote_id == 0 ? OUTPRIZE_DEFAULT_PREFIX : (remote_id & 0x7FF);
  const uint64_t full_frame = ((static_cast<uint64_t>(prefix & 0x7FF)) << 24) | (low24 & 0xFFFFFFULL);
  return this->transmit_full35_mode_(full_frame, repeats, mode, label);
}

bool RFBridgeComponent::transmit_full35_mode_(uint64_t full35, uint8_t repeats, TxFrameMode mode, const char *label) {
  if (!this->cc1101_configured_ || this->gdo0_pin_ == nullptr) {
    ESP_LOGE(TAG, "OUTPRIZE TX unavailable; CC1101 configured=%s gdo0=%s", YESNO(this->cc1101_configured_),
             this->gdo0_pin_ == nullptr ? "missing" : "present");
    return false;
  }

  const uint64_t full_frame = full35 & 0x7FFFFFFFFULL;
  const uint32_t prefix = static_cast<uint32_t>((full_frame >> 24) & 0x7FF);
  const uint32_t low24 = static_cast<uint32_t>(full_frame & 0xFFFFFFULL);
  if (repeats == 0) {
    repeats = 1;
  }
  if (repeats > 12) {
    repeats = 12;
  }

  uint8_t ones = 0;
  for (uint8_t i = 0; i < OUTPRIZE_TX_BITS; i++) {
    const uint8_t source_bit = (mode == TxFrameMode::LSB_NORMAL || mode == TxFrameMode::LSB_INVERTED) ? i : (OUTPRIZE_TX_BITS - 1 - i);
    bool bit_value = ((full_frame >> source_bit) & 0x01ULL) != 0;
    if (mode == TxFrameMode::MSB_INVERTED || mode == TxFrameMode::LSB_INVERTED) {
      bit_value = !bit_value;
    }
    if (bit_value) ones++;
  }
  const uint8_t zeros = OUTPRIZE_TX_BITS - ones;
  this->tx_log_frame_bits_(full_frame, OUTPRIZE_TX_BITS, mode, label);
  const uint32_t frame_data_duration_us =
      (static_cast<uint32_t>(zeros) * (OUTPRIZE_TX_PULSE_US + OUTPRIZE_TX_ZERO_GAP_US)) +
      (static_cast<uint32_t>(ones) * (OUTPRIZE_TX_PULSE_US + OUTPRIZE_TX_ONE_GAP_US));
  const uint32_t burst_duration_us = OUTPRIZE_TX_HEADER_ON_US + OUTPRIZE_TX_HEADER_OFF_US +
      (frame_data_duration_us * repeats) +
      (static_cast<uint32_t>(OUTPRIZE_TX_INTER_FRAME_GAP_US) * repeats);
  ESP_LOGI(TAG, "OUTPRIZE TX start label=%s prefix=0x%03X low24=0x%06X full35=0x%09llX repeats=%u bits=%u ones=%u zeros=%u timing[pulse=%u zero=%u one=%u header_on=%u header_off=%u gap=%u] est_duration=%u us",
           label, prefix, low24 & 0xFFFFFF, static_cast<unsigned long long>(full_frame), repeats, OUTPRIZE_TX_BITS, ones, zeros,
           OUTPRIZE_TX_PULSE_US, OUTPRIZE_TX_ZERO_GAP_US, OUTPRIZE_TX_ONE_GAP_US,
           OUTPRIZE_TX_HEADER_ON_US, OUTPRIZE_TX_HEADER_OFF_US, OUTPRIZE_TX_INTER_FRAME_GAP_US,
           burst_duration_us);

  // Pause RX while transmitting.  GDO0 normally acts as the CC1101 async data
  // output during receive; for async transmit we drive the same line from the ESP.
  this->rx_enabled_ = false;
  this->tx_log_marcstate_("before idle");
  this->cc1101_configure_ook_async_tx_();
  this->tx_dump_status_("after tx config");
  const bool calibrated = this->cc1101_calibrate_for_tx_();
  if (!calibrated) {
    ESP_LOGW(TAG, "CC1101 TX calibration did not report IDLE before STX; continuing for diagnostics");
  }
  this->tx_dump_status_("after SCAL");
  ESP_LOGI(TAG, "GDO0 direction: ESP output -> CC1101 async TX data input");
  this->gdo0_pin_->pin_mode(gpio::FLAG_OUTPUT);
  this->tx_write_carrier_(false);  // carrier off while entering TX
  delayMicroseconds(2000);
  const uint8_t stx_status = this->cc1101_strobe_(cc1101::STX);
  ESP_LOGI(TAG, "CC1101 TX strobe STX status=0x%02X", stx_status);
  delayMicroseconds(1000);
  this->tx_dump_status_("after STX");

  // v1.3.16: match the learned OEM frame header instead of using the earlier
  // synthetic reset/sync delimiter.  The learned Power Off capture starts with
  // two long edge intervals before the PWM payload: about 4.6 ms carrier-on,
  // then about 4.5 ms carrier-off, then the first data pulse.
  this->tx_write_carrier_(true);  // carrier on / OEM header lead-in
  delayMicroseconds(OUTPRIZE_TX_HEADER_ON_US);
  this->tx_write_carrier_(false); // carrier off / OEM header space
  delayMicroseconds(OUTPRIZE_TX_HEADER_OFF_US);

  for (uint8_t i = 0; i < repeats; i++) {
    this->tx_send_frame_bits_(full_frame, OUTPRIZE_TX_BITS, mode);
    delayMicroseconds(OUTPRIZE_TX_INTER_FRAME_GAP_US);
  }

  this->tx_write_carrier_(false);  // carrier off before restore
  delayMicroseconds(1000);
  this->tx_log_marcstate_("before idle restore");
  this->cc1101_enter_idle_();
  this->tx_log_marcstate_("after idle restore");

  // Restore RX mode and GDO0 input.
  ESP_LOGI(TAG, "GDO0 direction: ESP input <- CC1101 async RX data output");
  this->gdo0_pin_->pin_mode(gpio::FLAG_INPUT);
  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();
  this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
  this->rx_last_capture_ms_ = millis();
  this->rx_enabled_ = true;

  ESP_LOGI(TAG, "OUTPRIZE TX complete label=%s low24=0x%06X", label, low24 & 0xFFFFFF);
  return true;
}



uint16_t RFBridgeComponent::tx_build_edge_deltas_(uint64_t frame, uint8_t bits, TxFrameMode mode, bool include_preamble, uint16_t *out, uint16_t max_edges) const {
  uint16_t n = 0;
  auto add = [&](uint16_t v) {
    if (n < max_edges) out[n++] = v;
  };

  if (include_preamble) {
    add(OUTPRIZE_TX_HEADER_ON_US);
    add(OUTPRIZE_TX_HEADER_OFF_US);
  }

  for (uint8_t i = 0; i < bits; i++) {
    const uint8_t source_bit = (mode == TxFrameMode::LSB_NORMAL || mode == TxFrameMode::LSB_INVERTED) ? i : (bits - 1 - i);
    bool one = ((frame >> source_bit) & 0x01ULL) != 0;
    if (mode == TxFrameMode::MSB_INVERTED || mode == TxFrameMode::LSB_INVERTED) {
      one = !one;
    }
    add(OUTPRIZE_TX_PULSE_US);
    add(one ? OUTPRIZE_TX_ONE_GAP_US : OUTPRIZE_TX_ZERO_GAP_US);
  }
  return n;
}

void RFBridgeComponent::log_outprize_edge_compare_(uint64_t frame, uint8_t bits, TxFrameMode mode) const {
  if (!this->outprize_learned_valid_ || this->outprize_learned_edge_count_ == 0) {
    ESP_LOGW(TAG, "OUTPRIZE_COMPARE unavailable; press the OEM remote and wait for OUTPRIZE_LEARNED first");
    return;
  }

  uint16_t tx_data[96]{};
  uint16_t tx_full[100]{};
  const uint16_t tx_data_count = this->tx_build_edge_deltas_(frame, bits, mode, false, tx_data, 96);
  const uint16_t tx_full_count = this->tx_build_edge_deltas_(frame, bits, mode, true, tx_full, 100);

  const uint16_t start = this->outprize_learned_start_index_;
  const uint16_t stop = this->outprize_learned_stop_index_;
  const uint16_t oem_data_count = (stop >= start) ? static_cast<uint16_t>(stop - start + 1) : 0;
  const uint16_t cmp = (oem_data_count < tx_data_count) ? oem_data_count : tx_data_count;

  uint32_t abs_sum = 0;
  uint16_t max_abs = 0;
  uint16_t over_150 = 0;
  for (uint16_t i = 0; i < cmp; i++) {
    const uint16_t oem = this->outprize_learned_edges_[start + i];
    const uint16_t tx = tx_data[i];
    const uint16_t diff = oem > tx ? oem - tx : tx - oem;
    abs_sum += diff;
    if (diff > max_abs) max_abs = diff;
    if (diff > 150) over_150++;
  }

  ESP_LOGI(TAG, "===== OUTPRIZE_COMPARE =====");
  ESP_LOGI(TAG, "Learned capture=%u full35=0x%09llX bits=%u stream=%s",
           this->outprize_learned_capture_no_, static_cast<unsigned long long>(this->outprize_learned_full35_),
           this->outprize_learned_bits_, this->outprize_learned_binary_);
  ESP_LOGI(TAG, "OEM edges: total=%u data_start=%u data_stop=%u data_count=%u",
           this->outprize_learned_edge_count_, start, stop, oem_data_count);
  ESP_LOGI(TAG, "TX simulated: data_count=%u full_with_header_count=%u timing[pulse=%u zero_gap=%u one_gap=%u header_on=%u header_off=%u inter_gap=%u]",
           tx_data_count, tx_full_count, OUTPRIZE_TX_PULSE_US, OUTPRIZE_TX_ZERO_GAP_US, OUTPRIZE_TX_ONE_GAP_US,
           OUTPRIZE_TX_HEADER_ON_US, OUTPRIZE_TX_HEADER_OFF_US, OUTPRIZE_TX_INTER_FRAME_GAP_US);
  ESP_LOGI(TAG, "Data comparison: compared=%u avg_abs_error=%u us max_abs_error=%u us over_150us=%u",
           cmp, cmp == 0 ? 0 : static_cast<unsigned>(abs_sum / cmp), max_abs, over_150);

  for (uint16_t row = 0; row < cmp && row < 72; row += 8) {
    char line[220];
    uint16_t pos = 0;
    pos += snprintf(line + pos, sizeof(line) - pos, "  idx OEM/TX/diff [%02u-%02u]:", row, static_cast<unsigned>(row + 7 < cmp ? row + 7 : cmp - 1));
    for (uint16_t j = 0; j < 8 && row + j < cmp; j++) {
      const uint16_t oem = this->outprize_learned_edges_[start + row + j];
      const uint16_t tx = tx_data[row + j];
      const uint16_t diff = oem > tx ? oem - tx : tx - oem;
      pos += snprintf(line + pos, sizeof(line) - pos, " %u/%u/%u", oem, tx, diff);
    }
    ESP_LOGI(TAG, "%s", line);
  }

  const uint16_t pre_start = start >= 4 ? start - 4 : 0;
  char pre[220];
  uint16_t p = 0;
  p += snprintf(pre + p, sizeof(pre) - p, "OEM around frame start [%u-%u]:", pre_start, start + 5 < this->outprize_learned_edge_count_ ? start + 5 : this->outprize_learned_edge_count_ - 1);
  for (uint16_t i = pre_start; i < this->outprize_learned_edge_count_ && i <= start + 5; i++) {
    p += snprintf(pre + p, sizeof(pre) - p, " %u", this->outprize_learned_edges_[i]);
  }
  ESP_LOGI(TAG, "%s", pre);
  ESP_LOGI(TAG, "TX with OEM header first edges: %u %u %u %u %u %u %u %u",
           tx_full_count > 0 ? tx_full[0] : 0, tx_full_count > 1 ? tx_full[1] : 0,
           tx_full_count > 2 ? tx_full[2] : 0, tx_full_count > 3 ? tx_full[3] : 0,
           tx_full_count > 4 ? tx_full[4] : 0, tx_full_count > 5 ? tx_full[5] : 0,
           tx_full_count > 6 ? tx_full[6] : 0, tx_full_count > 7 ? tx_full[7] : 0);
  ESP_LOGI(TAG, "============================");
}

void RFBridgeComponent::compare_last_outprize_learned() {
  if (!this->outprize_learned_valid_) {
    ESP_LOGW(TAG, "OUTPRIZE_COMPARE unavailable; no learned frame");
    return;
  }
  this->log_outprize_edge_compare_(this->outprize_learned_full35_, OUTPRIZE_TX_BITS, TxFrameMode::MSB_NORMAL);
}


void RFBridgeComponent::set_outprize_learned_frame(uint64_t full35) {
  const uint64_t full_frame = full35 & 0x7FFFFFFFFULL;
  this->outprize_learned_valid_ = true;
  this->outprize_learned_full35_ = full_frame;
  this->outprize_learned_low24_ = static_cast<uint32_t>(full_frame & 0xFFFFFFULL);
  this->outprize_learned_remote_id_ = static_cast<uint16_t>((full_frame >> 24) & 0x7FFULL);
  this->outprize_learned_bits_ = OUTPRIZE_TX_BITS;
  this->outprize_learned_capture_no_ = 0;
  this->outprize_learned_score_ = 0;
  this->outprize_learned_start_index_ = 0;
  this->outprize_learned_stop_index_ = 0;
  this->outprize_learned_edge_count_ = 0;
  this->outprize_learned_initial_level_ = 0;
  memset(this->outprize_learned_edges_, 0, sizeof(this->outprize_learned_edges_));
  memset(this->outprize_learned_levels_, 0, sizeof(this->outprize_learned_levels_));

  for (uint8_t i = 0; i < OUTPRIZE_TX_BITS; i++) {
    const uint8_t bit_index = OUTPRIZE_TX_BITS - 1 - i;
    this->outprize_learned_binary_[i] = ((full_frame >> bit_index) & 0x01ULL) ? '1' : '0';
  }
  this->outprize_learned_binary_[OUTPRIZE_TX_BITS] = '\0';

  ESP_LOGI(TAG, "OUTPRIZE_LEARNED manually set full35=0x%09llX remote=0x%03X low24=0x%06X bits=%u stream=%s",
           static_cast<unsigned long long>(this->outprize_learned_full35_),
           this->outprize_learned_remote_id_, this->outprize_learned_low24_,
           this->outprize_learned_bits_, this->outprize_learned_binary_);
}

bool RFBridgeComponent::replay_known_outprize_power_off(uint8_t repeats) {
  const uint64_t full35 = ((static_cast<uint64_t>(OUTPRIZE_DEFAULT_PREFIX) << 24) | 0x600000ULL);
  ESP_LOGI(TAG, "OUTPRIZE known Power Off replay full35=0x%09llX remote=0x%03X low24=0x600000 repeats=%u",
           static_cast<unsigned long long>(full35), OUTPRIZE_DEFAULT_PREFIX, repeats);
  return this->transmit_full35_mode_(full35, repeats, TxFrameMode::MSB_NORMAL, "KNOWN_POWER_OFF_FULL35_MSB_NORMAL");
}

void RFBridgeComponent::compare_known_outprize_power_off() {
  const uint64_t full35 = ((static_cast<uint64_t>(OUTPRIZE_DEFAULT_PREFIX) << 24) | 0x600000ULL);
  ESP_LOGI(TAG, "OUTPRIZE known Power Off compare requested full35=0x%09llX remote=0x%03X low24=0x600000",
           static_cast<unsigned long long>(full35), OUTPRIZE_DEFAULT_PREFIX);
  if (this->outprize_learned_valid_ && this->outprize_learned_edge_count_ > 0) {
    this->log_outprize_edge_compare_(full35, OUTPRIZE_TX_BITS, TxFrameMode::MSB_NORMAL);
    return;
  }

  uint16_t edges[96]{};
  const uint16_t count = this->tx_build_edge_deltas_(full35, OUTPRIZE_TX_BITS, TxFrameMode::MSB_NORMAL, true, edges, 96);
  ESP_LOGI(TAG, "===== OUTPRIZE_KNOWN_POWER_OFF_TX_TIMING =====");
  ESP_LOGI(TAG, "No learned OEM edge capture is available, so this logs the generated known-frame TX timing only.");
  ESP_LOGI(TAG, "full35=0x%09llX bits=%u edge_count=%u timing[pulse=%u zero=%u one=%u header_on=%u header_off=%u gap=%u]",
           static_cast<unsigned long long>(full35), OUTPRIZE_TX_BITS, count,
           OUTPRIZE_TX_PULSE_US, OUTPRIZE_TX_ZERO_GAP_US, OUTPRIZE_TX_ONE_GAP_US,
           OUTPRIZE_TX_HEADER_ON_US, OUTPRIZE_TX_HEADER_OFF_US, OUTPRIZE_TX_INTER_FRAME_GAP_US);
  char line[180];
  for (uint16_t i = 0; i < count; i += 8) {
    const uint16_t end = (i + 7 < count) ? (i + 7) : (count - 1);
    int pos = snprintf(line, sizeof(line), "  tx_edges[%03u-%03u]", static_cast<unsigned>(i), static_cast<unsigned>(end));
    for (uint16_t j = i; j < count && j < i + 8 && pos > 0 && pos < static_cast<int>(sizeof(line)); j++) {
      pos += snprintf(line + pos, sizeof(line) - pos, " %u", edges[j]);
    }
    ESP_LOGI(TAG, "%s", line);
  }
  ESP_LOGI(TAG, "===============================================");
}

bool RFBridgeComponent::send_ook_test_burst(uint16_t pulse_us, uint16_t pulse_count, uint8_t repeats) {
  return this->transmit_ook_test_burst_(pulse_us, pulse_count, repeats);
}


bool RFBridgeComponent::send_ook_carrier_test(uint16_t duration_ms) {
  return this->start_ook_carrier_test_(duration_ms);
}

bool RFBridgeComponent::start_ook_carrier_test_(uint16_t duration_ms) {
  if (!this->cc1101_configured_ || this->gdo0_pin_ == nullptr) {
    ESP_LOGE(TAG, "OOK carrier TX test unavailable; CC1101 configured=%s gdo0=%s", YESNO(this->cc1101_configured_),
             this->gdo0_pin_ == nullptr ? "missing" : "present");
    return false;
  }
  if (this->tx_carrier_active_) {
    ESP_LOGW(TAG, "OOK carrier TX test already active; ignoring new request");
    return false;
  }

  if (duration_ms < 100) duration_ms = 100;
  if (duration_ms > 5000) duration_ms = 5000;

  ESP_LOGI(TAG, "OOK TX carrier test scheduled duration=%u ms (non-blocking)", duration_ms);

  this->rx_enabled_ = false;
  this->tx_log_marcstate_("carrier before idle");
  this->cc1101_configure_ook_async_tx_();
  this->tx_dump_status_("carrier after tx config");
  const bool calibrated = this->cc1101_calibrate_for_tx_();
  if (!calibrated) {
    ESP_LOGW(TAG, "CC1101 TX calibration did not report IDLE before carrier STX; continuing for diagnostics");
  }
  this->tx_dump_status_("carrier after SCAL");
  ESP_LOGI(TAG, "GDO0 direction: ESP output -> CC1101 async TX data input");
  this->gdo0_pin_->pin_mode(gpio::FLAG_OUTPUT);

  // In CC1101 async OOK TX, GDO0 is the serial/envelope data input.
  // Keep it high while the non-blocking carrier state machine is active.
  ESP_LOGI(TAG, "GDO0 TX data forced HIGH for carrier envelope");
  this->tx_write_data_(true);
  delayMicroseconds(2000);
  const uint8_t stx_status = this->cc1101_strobe_(cc1101::STX);
  ESP_LOGI(TAG, "CC1101 TX carrier STX status=0x%02X", stx_status);
  delayMicroseconds(1000);
  this->tx_dump_status_("carrier after STX");

  this->tx_carrier_started_ms_ = millis();
  this->tx_carrier_duration_ms_ = duration_ms;
  this->tx_carrier_active_ = true;
  ESP_LOGI(TAG, "OOK TX carrier active; will restore RX in %u ms", duration_ms);
  return true;
}

void RFBridgeComponent::tx_carrier_loop_() {
  if (!this->tx_carrier_active_) {
    return;
  }
  if (millis() - this->tx_carrier_started_ms_ < this->tx_carrier_duration_ms_) {
    return;
  }
  this->finish_ook_carrier_test_();
}

void RFBridgeComponent::finish_ook_carrier_test_() {
  if (!this->tx_carrier_active_) {
    return;
  }

  const uint16_t duration_ms = this->tx_carrier_duration_ms_;
  this->tx_carrier_active_ = false;
  ESP_LOGI(TAG, "GDO0 TX data forced LOW before carrier restore");
  this->tx_write_data_(false);
  delayMicroseconds(1000);
  this->tx_log_marcstate_("carrier before idle restore");
  this->cc1101_enter_idle_();
  this->tx_log_marcstate_("carrier after idle restore");

  ESP_LOGI(TAG, "GDO0 direction: ESP input <- CC1101 async RX data output");
  this->gdo0_pin_->pin_mode(gpio::FLAG_INPUT);
  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();
  this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
  this->rx_last_capture_ms_ = millis();
  this->rx_enabled_ = true;

  ESP_LOGI(TAG, "OOK TX carrier test complete duration=%u ms", duration_ms);
}

bool RFBridgeComponent::transmit_ook_test_burst_(uint16_t pulse_us, uint16_t pulse_count, uint8_t repeats) {
  if (!this->cc1101_configured_ || this->gdo0_pin_ == nullptr) {
    ESP_LOGE(TAG, "OOK TX test unavailable; CC1101 configured=%s gdo0=%s", YESNO(this->cc1101_configured_),
             this->gdo0_pin_ == nullptr ? "missing" : "present");
    return false;
  }

  if (pulse_us < 100) pulse_us = 100;
  if (pulse_us > 5000) pulse_us = 5000;
  if (pulse_count < 2) pulse_count = 2;
  if (pulse_count > 400) pulse_count = 400;
  if (repeats == 0) repeats = 1;
  if (repeats > 10) repeats = 10;

  const uint32_t total_duration_us = (static_cast<uint32_t>(pulse_us) * pulse_count + 10000UL) * repeats;
  ESP_LOGI(TAG, "OOK TX hardware test start pulse=%u us count=%u repeats=%u est_duration=%u us",
           pulse_us, pulse_count, repeats, total_duration_us);

  this->rx_enabled_ = false;
  this->tx_log_marcstate_("before idle");
  this->cc1101_configure_ook_async_tx_();
  this->tx_dump_status_("after tx config");
  const bool calibrated = this->cc1101_calibrate_for_tx_();
  if (!calibrated) {
    ESP_LOGW(TAG, "CC1101 TX calibration did not report IDLE before STX; continuing for diagnostics");
  }
  this->tx_dump_status_("after SCAL");
  ESP_LOGI(TAG, "GDO0 direction: ESP output -> CC1101 async TX data input");
  this->gdo0_pin_->pin_mode(gpio::FLAG_OUTPUT);

  ESP_LOGI(TAG, "GDO0 TX data forced LOW before test burst");
  this->tx_write_data_(false);
  delayMicroseconds(2000);
  const uint8_t stx_status = this->cc1101_strobe_(cc1101::STX);
  ESP_LOGI(TAG, "CC1101 TX STX status=0x%02X", stx_status);
  delayMicroseconds(1000);
  this->tx_dump_status_("after STX");

  for (uint8_t r = 0; r < repeats; r++) {
    for (uint16_t i = 0; i < pulse_count; i++) {
      this->tx_write_data_((i & 1) == 0);
      delayMicroseconds(pulse_us);
    }
    this->tx_write_data_(false);
    delayMicroseconds(10000);
  }

  this->tx_write_data_(false);
  delayMicroseconds(1000);
  this->tx_log_marcstate_("before idle restore");
  this->cc1101_enter_idle_();
  this->tx_log_marcstate_("after idle restore");

  ESP_LOGI(TAG, "GDO0 direction: ESP input <- CC1101 async RX data output");
  this->gdo0_pin_->pin_mode(gpio::FLAG_INPUT);
  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();
  this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
  this->rx_last_capture_ms_ = millis();
  this->rx_enabled_ = true;

  ESP_LOGI(TAG, "OOK TX hardware test complete pulse=%u us count=%u repeats=%u", pulse_us, pulse_count, repeats);
  return true;
}

bool RFBridgeComponent::replay_last_capture(uint8_t repeats) {
  return this->transmit_last_capture_(repeats);
}

bool RFBridgeComponent::replay_last_outprize_learned(uint8_t repeats) {
  return this->transmit_learned_outprize_(repeats);
}

void RFBridgeComponent::clear_last_outprize_learned() {
  this->outprize_learned_valid_ = false;
  this->outprize_learned_full35_ = 0;
  this->outprize_learned_low24_ = 0;
  this->outprize_learned_remote_id_ = 0;
  this->outprize_learned_bits_ = 0;
  this->outprize_learned_capture_no_ = 0;
  this->outprize_learned_score_ = 0;
  this->outprize_learned_start_index_ = 0;
  this->outprize_learned_stop_index_ = 0;
  this->outprize_learned_edge_count_ = 0;
  this->outprize_learned_initial_level_ = 0;
  memset(this->outprize_learned_edges_, 0, sizeof(this->outprize_learned_edges_));
  memset(this->outprize_learned_levels_, 0, sizeof(this->outprize_learned_levels_));
  this->outprize_learned_binary_[0] = '\0';
  ESP_LOGI(TAG, "OUTPRIZE_LEARNED cleared");
}

bool RFBridgeComponent::transmit_learned_outprize_(uint8_t repeats) {
  if (!this->outprize_learned_valid_) {
    ESP_LOGW(TAG, "OUTPRIZE learned replay unavailable; press the OEM remote first and wait for OUTPRIZE_LEARNED in the log");
    return false;
  }
  ESP_LOGI(TAG, "OUTPRIZE learned replay start capture=%u full35=0x%09llX remote=0x%03X low24=0x%06X bits=%u score=%u stream=%s",
           this->outprize_learned_capture_no_, static_cast<unsigned long long>(this->outprize_learned_full35_),
           this->outprize_learned_remote_id_, this->outprize_learned_low24_,
           this->outprize_learned_bits_, this->outprize_learned_score_, this->outprize_learned_binary_);
  return this->transmit_full35_mode_(this->outprize_learned_full35_, repeats, TxFrameMode::MSB_NORMAL, "LEARNED_FULL35_MSB_NORMAL");
}


std::string RFBridgeComponent::get_last_learned_summary() const {
  if (!this->outprize_learned_valid_) {
    return std::string("No command learned");
  }
  const char *name = "Unknown";
  switch (this->outprize_learned_low24_ & 0xFFFFFFUL) {
    case 0x600000: name = "Power Off"; break;
    case 0x600040: name = "Fan Awake"; break;
    case 0x600160: name = "Vent Close"; break;
    case 0x600350: name = "Vent Open"; break;
    case 0x600154: name = "Rain Sensor"; break;
    case 0x600140: name = "40% Out"; break;
    case 0x600240: name = "10% Out"; break;
    case 0x600440: name = "20% Out"; break;
    case 0x600640: name = "30% Out"; break;
    case 0x600340: name = "50% Out"; break;
    case 0x600540: name = "60% Out"; break;
    case 0x600740: name = "70% Out"; break;
    case 0x600170: name = "80% Out"; break;
    case 0x600178: name = "90% Out"; break;
    case 0x60017C: name = "100% Out"; break;
  }
  char buf[160];
  snprintf(buf, sizeof(buf), "%s low24=0x%06X remote=0x%03X bits=%u edges=%u cap=%u",
           name, this->outprize_learned_low24_, this->outprize_learned_remote_id_,
           this->outprize_learned_bits_, this->outprize_learned_edge_count_, this->outprize_learned_capture_no_);
  return std::string(buf);
}

std::string RFBridgeComponent::get_sequence_summary() const {
  if (this->sequence_count_ == 0) {
    return this->sequence_active_ ? std::string("Learning sequence...") : std::string("No sequence captured");
  }
  return std::string(this->sequence_last_summary_);
}

void RFBridgeComponent::start_rf_sequence_capture(uint16_t duration_ms) {
  if (duration_ms < 300) duration_ms = 300;
  if (duration_ms > 5000) duration_ms = 5000;
  this->clear_rf_sequence_capture();
  // Deliberate/gated learning: starting a new learn window invalidates the
  // previous learned command, and normal RF traffic outside this window does
  // not overwrite the learned command.
  this->clear_last_outprize_learned();
  this->sequence_active_ = true;
  this->sequence_started_ms_ = millis();
  this->sequence_until_ms_ = this->sequence_started_ms_ + duration_ms;
  snprintf(this->sequence_last_summary_, sizeof(this->sequence_last_summary_), "Learning sequence for %u ms", duration_ms);
  ESP_LOGI(TAG, "RF_SEQUENCE_LEARN start duration=%u ms max_frames=%u", duration_ms, RF_SEQUENCE_MAX_FRAMES);
}

void RFBridgeComponent::clear_rf_sequence_capture() {
  this->sequence_active_ = false;
  this->sequence_started_ms_ = 0;
  this->sequence_until_ms_ = 0;
  this->sequence_count_ = 0;
  memset(this->sequence_capture_no_, 0, sizeof(this->sequence_capture_no_));
  memset(this->sequence_frame_ms_, 0, sizeof(this->sequence_frame_ms_));
  memset(this->sequence_edge_count_, 0, sizeof(this->sequence_edge_count_));
  memset(this->sequence_initial_level_, 0, sizeof(this->sequence_initial_level_));
  memset(this->sequence_decoded_, 0, sizeof(this->sequence_decoded_));
  memset(this->sequence_full35_, 0, sizeof(this->sequence_full35_));
  memset(this->sequence_low24_, 0, sizeof(this->sequence_low24_));
  memset(this->sequence_remote_id_, 0, sizeof(this->sequence_remote_id_));
  memset(this->sequence_edges_, 0, sizeof(this->sequence_edges_));
  memset(this->sequence_levels_, 0, sizeof(this->sequence_levels_));
  snprintf(this->sequence_last_summary_, sizeof(this->sequence_last_summary_), "No sequence captured");
  ESP_LOGI(TAG, "RF_SEQUENCE cleared");
}

void RFBridgeComponent::sequence_store_current_capture_(uint32_t capture_no, bool decoded) {
  if (this->sequence_active_ && static_cast<int32_t>(millis() - this->sequence_until_ms_) > 0) {
    this->sequence_active_ = false;
    ESP_LOGI(TAG, "RF_SEQUENCE_LEARN timeout frames=%u", this->sequence_count_);
  }
  if (!this->sequence_active_) return;
  if (this->sequence_count_ >= RF_SEQUENCE_MAX_FRAMES) {
    this->sequence_active_ = false;
    ESP_LOGW(TAG, "RF_SEQUENCE_LEARN full frames=%u", this->sequence_count_);
    return;
  }
  const uint8_t idx = this->sequence_count_++;
  this->sequence_capture_no_[idx] = capture_no;
  this->sequence_frame_ms_[idx] = millis() - this->sequence_started_ms_;
  this->sequence_edge_count_[idx] = this->rx_edge_count_;
  this->sequence_initial_level_[idx] = this->rx_capture_initial_level_;
  this->sequence_decoded_[idx] = decoded && this->outprize_learned_capture_no_ == capture_no;
  if (this->sequence_decoded_[idx]) {
    this->sequence_full35_[idx] = this->outprize_learned_full35_;
    this->sequence_low24_[idx] = this->outprize_learned_low24_;
    this->sequence_remote_id_[idx] = this->outprize_learned_remote_id_;
  }
  for (uint16_t i = 0; i < this->rx_edge_count_ && i < RX_MAX_EDGES; i++) {
    this->sequence_edges_[idx][i] = this->rx_edges_[i];
    this->sequence_levels_[idx][i] = this->rx_levels_[i];
  }
  snprintf(this->sequence_last_summary_, sizeof(this->sequence_last_summary_),
           "%u frame(s), last %s low24=0x%06X edges=%u",
           this->sequence_count_, this->sequence_decoded_[idx] ? "Outprize" : "raw",
           this->sequence_low24_[idx], this->sequence_edge_count_[idx]);
  ESP_LOGI(TAG, "RF_SEQUENCE_FRAME[%u] t=%u ms capture=%u decoded=%s low24=0x%06X edges=%u",
           idx + 1, this->sequence_frame_ms_[idx], capture_no, YESNO(this->sequence_decoded_[idx]),
           this->sequence_low24_[idx], this->sequence_edge_count_[idx]);
}

bool RFBridgeComponent::replay_rf_sequence_raw(uint8_t repeats) {
  return this->transmit_sequence_raw_(repeats);
}

bool RFBridgeComponent::transmit_sequence_raw_(uint8_t repeats) {
  if (this->sequence_count_ == 0) {
    ESP_LOGW(TAG, "RF_SEQUENCE replay unavailable; no sequence captured");
    return false;
  }
  ESP_LOGI(TAG, "RF_SEQUENCE replay start frames=%u repeats=%u", this->sequence_count_, repeats);
  bool ok = true;
  for (uint8_t r = 0; r < repeats; r++) {
    for (uint8_t i = 0; i < this->sequence_count_; i++) {
      char label[32];
      snprintf(label, sizeof(label), "SEQ_%u", i + 1);
      ok &= this->transmit_raw_edge_capture_(this->sequence_edges_[i], this->sequence_levels_[i],
                                             this->sequence_edge_count_[i], this->sequence_initial_level_[i], 1, label);
      if (i + 1 < this->sequence_count_) delay(50);
    }
  }
  ESP_LOGI(TAG, "RF_SEQUENCE replay complete frames=%u ok=%s", this->sequence_count_, YESNO(ok));
  return ok;
}


bool RFBridgeComponent::replay_last_outprize_raw_capture(uint8_t repeats) {
  return this->transmit_learned_raw_outprize_(repeats);
}

bool RFBridgeComponent::transmit_learned_raw_outprize_(uint8_t repeats) {
  if (!this->outprize_learned_valid_ || this->outprize_learned_edge_count_ < OUTPRIZE_MIN_EDGES) {
    ESP_LOGW(TAG, "OUTPRIZE raw replay unavailable; press the OEM remote and wait for OUTPRIZE_LEARNED first (valid=%s edges=%u)",
             YESNO(this->outprize_learned_valid_), this->outprize_learned_edge_count_);
    return false;
  }
  ESP_LOGI(TAG, "OUTPRIZE raw learned replay start capture=%u full35=0x%09llX edges=%u initial_gdo0=%u repeats=%u start=%u stop=%u",
           this->outprize_learned_capture_no_, static_cast<unsigned long long>(this->outprize_learned_full35_),
           this->outprize_learned_edge_count_, this->outprize_learned_initial_level_, repeats,
           this->outprize_learned_start_index_, this->outprize_learned_stop_index_);
  return this->transmit_raw_edge_capture_(this->outprize_learned_edges_, this->outprize_learned_levels_,
                                          this->outprize_learned_edge_count_, this->outprize_learned_initial_level_,
                                          repeats, "LEARNED_RAW_EDGES");
}

bool RFBridgeComponent::transmit_raw_edge_capture_(const uint16_t *edges, const uint8_t *levels, uint16_t edge_count,
                                                   uint8_t initial_level, uint8_t repeats, const char *label) {
  if (!this->cc1101_configured_ || this->gdo0_pin_ == nullptr) {
    ESP_LOGE(TAG, "RF raw replay unavailable; CC1101 configured=%s gdo0=%s", YESNO(this->cc1101_configured_),
             this->gdo0_pin_ == nullptr ? "missing" : "present");
    return false;
  }
  if (edges == nullptr || levels == nullptr || edge_count < RX_MIN_EDGES) {
    ESP_LOGW(TAG, "RF raw replay unavailable; no usable edge capture stored (edges=%u)", edge_count);
    return false;
  }
  if (repeats == 0) repeats = 1;
  if (repeats > 4) repeats = 4;

  uint32_t single_duration = 0;
  for (uint16_t i = 0; i < edge_count; i++) single_duration += edges[i];
  ESP_LOGI(TAG, "RF raw replay start label=%s edges=%u initial_gdo0=%u repeats=%u single_duration=%u us gap=%u us",
           label, edge_count, initial_level, repeats, single_duration, OUTPRIZE_TX_INTER_FRAME_GAP_US);

  if (this->diagnostic_logging_) {
    char line[220];
    for (uint16_t row = 0; row < edge_count && row < 96; row += 8) {
      int pos = snprintf(line, sizeof(line), "  raw_edges[%03u-%03u]", static_cast<unsigned>(row),
                         static_cast<unsigned>((row + 7 < edge_count) ? row + 7 : edge_count - 1));
      for (uint16_t j = row; j < edge_count && j < row + 8 && pos > 0 && pos < static_cast<int>(sizeof(line)); j++) {
        pos += snprintf(line + pos, sizeof(line) - pos, " %u/%u", edges[j], levels[j]);
      }
      ESP_LOGI(TAG, "%s", line);
    }
    if (edge_count > 96) ESP_LOGI(TAG, "  raw edge log truncated at 96 of %u edges", edge_count);
  }

  this->rx_enabled_ = false;
  this->tx_log_marcstate_("raw before idle");
  this->cc1101_configure_ook_async_tx_();
  this->tx_dump_status_("raw after tx config");
  const bool calibrated = this->cc1101_calibrate_for_tx_();
  if (!calibrated) {
    ESP_LOGW(TAG, "CC1101 TX calibration did not report IDLE before raw replay STX; continuing for diagnostics");
  }
  this->tx_dump_status_("raw after SCAL");
  ESP_LOGI(TAG, "GDO0 direction: ESP output -> CC1101 async TX data input");
  this->gdo0_pin_->pin_mode(gpio::FLAG_OUTPUT);

  bool current_level = initial_level != 0;
  this->tx_write_data_(current_level);
  delayMicroseconds(2000);
  const uint8_t stx_status = this->cc1101_strobe_(cc1101::STX);
  ESP_LOGI(TAG, "CC1101 TX raw replay STX status=0x%02X", stx_status);
  delayMicroseconds(1000);
  this->tx_dump_status_("raw after STX");

  for (uint8_t r = 0; r < repeats; r++) {
    current_level = initial_level != 0;
    this->tx_write_data_(current_level);
    for (uint16_t i = 0; i < edge_count; i++) {
      delayMicroseconds(edges[i]);
      current_level = levels[i] != 0;
      this->tx_write_data_(current_level);
    }
    this->tx_write_data_(false);
    delayMicroseconds(OUTPRIZE_TX_INTER_FRAME_GAP_US);
  }

  this->tx_write_data_(false);
  delayMicroseconds(1000);
  this->tx_log_marcstate_("raw before idle restore");
  this->cc1101_enter_idle_();
  this->tx_log_marcstate_("raw after idle restore");

  ESP_LOGI(TAG, "GDO0 direction: ESP input <- CC1101 async RX data output");
  this->gdo0_pin_->pin_mode(gpio::FLAG_INPUT);
  this->cc1101_configure_ook_async_rx_();
  this->cc1101_enter_rx_();
  this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
  this->rx_last_capture_ms_ = millis();
  this->rx_enabled_ = true;

  ESP_LOGI(TAG, "RF raw replay complete label=%s edges=%u repeats=%u", label, edge_count, repeats);
  return true;
}

bool RFBridgeComponent::transmit_last_capture_(uint8_t repeats) {
  if (this->rx_edge_count_ < RX_MIN_EDGES) {
    ESP_LOGW(TAG, "RF replay unavailable; no usable capture stored (edges=%u)", this->rx_edge_count_);
    return false;
  }
  uint16_t edges[RX_MAX_EDGES];
  uint8_t levels[RX_MAX_EDGES];
  const uint16_t edge_count = this->rx_edge_count_;
  for (uint16_t i = 0; i < edge_count; i++) {
    edges[i] = this->rx_edges_[i];
    levels[i] = this->rx_levels_[i];
  }
  return this->transmit_raw_edge_capture_(edges, levels, edge_count, this->rx_capture_initial_level_, repeats, "LAST_RAW_CAPTURE");
}


bool RFBridgeComponent::replay_last_outprize_raw_capture_stx882(uint8_t repeats) {
  if (!this->outprize_learned_valid_ || this->outprize_learned_edge_count_ < OUTPRIZE_MIN_EDGES) {
    ESP_LOGW(TAG, "STX882 learned raw replay unavailable; learn an OEM command first (valid=%s edges=%u)",
             YESNO(this->outprize_learned_valid_), this->outprize_learned_edge_count_);
    return false;
  }
  return this->transmit_raw_edge_capture_stx882_(this->outprize_learned_edges_, this->outprize_learned_levels_,
                                                 this->outprize_learned_edge_count_,
                                                 this->outprize_learned_initial_level_, repeats,
                                                 "STX882_LEARNED_RAW");
}

bool RFBridgeComponent::replay_rf_sequence_raw_stx882(uint8_t repeats) {
  return this->transmit_sequence_raw_stx882_(repeats);
}

bool RFBridgeComponent::send_outprize_power_off_stx882(uint8_t repeats) {
  uint16_t edges[96]{};
  const uint64_t frame = (static_cast<uint64_t>(this->outprize_remote_id_ & 0x7FF) << 24) | 0x600000ULL;
  const uint16_t count = this->tx_build_edge_deltas_(frame, OUTPRIZE_TX_BITS, TxFrameMode::MSB_NORMAL, true,
                                                     edges, sizeof(edges) / sizeof(edges[0]));
  if (count == 0) {
    ESP_LOGE(TAG, "STX882 Power Off waveform build failed");
    return false;
  }
  uint8_t levels[96]{};
  bool level = true;
  for (uint16_t i = 0; i < count; i++) {
    level = !level;
    levels[i] = level ? 1 : 0;
  }
  ESP_LOGI(TAG, "STX882 known Power Off TX full35=0x%09llX edges=%u repeats=%u",
           static_cast<unsigned long long>(frame), count, repeats);
  return this->transmit_raw_edge_capture_stx882_(edges, levels, count, 1, repeats, "STX882_POWER_OFF");
}

bool RFBridgeComponent::transmit_raw_edge_capture_stx882_(const uint16_t *edges, const uint8_t *levels,
                                                           uint16_t edge_count, uint8_t initial_level,
                                                           uint8_t repeats, const char *label) {
  if (this->stx882_data_pin_ == nullptr) {
    ESP_LOGE(TAG, "STX882 TX unavailable; stx882_data_pin is not configured");
    return false;
  }
  if (edges == nullptr || levels == nullptr || edge_count == 0) {
    ESP_LOGW(TAG, "STX882 TX unavailable; no usable edge capture stored (edges=%u)", edge_count);
    return false;
  }
  if (repeats == 0) repeats = 1;
  if (repeats > 8) repeats = 8;

  uint32_t single_duration = 0;
  for (uint16_t i = 0; i < edge_count; i++) single_duration += edges[i];
  ESP_LOGI(TAG, "STX882 raw TX start label=%s edges=%u initial=%u repeats=%u duration=%u us gap=%u us",
           label, edge_count, initial_level, repeats, single_duration, OUTPRIZE_TX_INTER_FRAME_GAP_US);

  // Keep the CC1101 quiet while the nearby STX882 transmits. This prevents
  // self-reception and front-end overload during normal operation.
  const bool restore_cc1101 = this->cc1101_configured_;
  if (restore_cc1101) {
    this->rx_enabled_ = false;
    this->cc1101_enter_idle_();
  }

  this->stx882_data_pin_->pin_mode(gpio::FLAG_OUTPUT);
  bool current_level = initial_level != 0;
  this->stx882_data_pin_->digital_write(current_level);
  delayMicroseconds(1000);

  for (uint8_t r = 0; r < repeats; r++) {
    current_level = initial_level != 0;
    this->stx882_data_pin_->digital_write(current_level);
    for (uint16_t i = 0; i < edge_count; i++) {
      delayMicroseconds(edges[i]);
      current_level = levels[i] != 0;
      this->stx882_data_pin_->digital_write(current_level);
    }
    this->stx882_data_pin_->digital_write(false);
    delayMicroseconds(OUTPRIZE_TX_INTER_FRAME_GAP_US);
  }
  this->stx882_data_pin_->digital_write(false);

  if (restore_cc1101) {
    this->cc1101_configure_ook_async_rx_();
    this->cc1101_enter_rx_();
    if (this->gdo0_pin_ != nullptr) {
      this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
    }
    this->rx_last_capture_ms_ = millis();
    this->rx_enabled_ = true;
  }

  ESP_LOGI(TAG, "STX882 raw TX complete label=%s edges=%u repeats=%u", label, edge_count, repeats);
  return true;
}

bool RFBridgeComponent::transmit_sequence_raw_stx882_(uint8_t repeats) {
  if (this->sequence_count_ == 0) {
    ESP_LOGW(TAG, "STX882 sequence replay unavailable; no sequence captured");
    return false;
  }
  if (repeats == 0) repeats = 1;
  if (repeats > 4) repeats = 4;
  ESP_LOGI(TAG, "STX882 sequence replay start frames=%u repeats=%u", this->sequence_count_, repeats);
  bool ok = true;
  for (uint8_t r = 0; r < repeats; r++) {
    for (uint8_t i = 0; i < this->sequence_count_; i++) {
      char label[40];
      snprintf(label, sizeof(label), "STX882_SEQUENCE_%u", i + 1);
      ok &= this->transmit_raw_edge_capture_stx882_(this->sequence_edges_[i], this->sequence_levels_[i],
                                                    this->sequence_edge_count_[i],
                                                    this->sequence_initial_level_[i], 1, label);
      if (i + 1 < this->sequence_count_) {
        uint32_t wait_ms = 50;
        if (this->sequence_frame_ms_[i + 1] > this->sequence_frame_ms_[i]) {
          wait_ms = this->sequence_frame_ms_[i + 1] - this->sequence_frame_ms_[i];
          if (wait_ms > 500) wait_ms = 500;
        }
        delay(wait_ms);
      }
    }
  }
  ESP_LOGI(TAG, "STX882 sequence replay complete frames=%u ok=%s", this->sequence_count_, YESNO(ok));
  return ok;
}

void RFBridgeComponent::capture_srx882_raw(uint16_t duration_ms) {
  if (this->srx882_data_pin_ == nullptr) {
    ESP_LOGE(TAG, "SRX882 capture unavailable; srx882_data_pin is not configured");
    return;
  }
  if (duration_ms < 250) duration_ms = 250;
  if (duration_ms > 5000) duration_ms = 5000;

  this->clear_srx882_capture();
  if (this->srx882_enable_pin_ != nullptr) this->srx882_enable_pin_->digital_write(true);
  if (this->cc1101_configured_) {
    this->rx_enabled_ = false;
    this->cc1101_enter_idle_();
  }

  this->srx882_data_pin_->pin_mode(gpio::FLAG_INPUT);
  bool last_level = this->srx882_data_pin_->digital_read();
  this->srx882_initial_level_ = last_level ? 1 : 0;
  const uint32_t start_us = micros();
  uint32_t last_edge_us = start_us;
  const uint32_t end_us = start_us + static_cast<uint32_t>(duration_ms) * 1000UL;

  ESP_LOGI(TAG, "SRX882 raw capture start duration=%u ms initial=%u; press the OEM remote now",
           duration_ms, this->srx882_initial_level_);
  while (static_cast<int32_t>(micros() - end_us) < 0) {
    const bool level = this->srx882_data_pin_->digital_read();
    if (level != last_level) {
      const uint32_t now = micros();
      if (this->srx882_edge_count_ < SRX882_MAX_EDGES) {
        uint32_t delta = now - last_edge_us;
        if (delta > 65535) delta = 65535;
        this->srx882_edges_[this->srx882_edge_count_] = static_cast<uint16_t>(delta);
        this->srx882_levels_[this->srx882_edge_count_] = level ? 1 : 0;
        this->srx882_edge_count_++;
      }
      last_edge_us = now;
      last_level = level;
    }
    delayMicroseconds(2);
  }
  this->srx882_capture_duration_us_ = micros() - start_us;
  this->srx882_capture_valid_ = this->srx882_edge_count_ >= RX_MIN_EDGES;
  snprintf(this->srx882_summary_, sizeof(this->srx882_summary_), "%s: %u edges / %u ms",
           this->srx882_capture_valid_ ? "Captured" : "No valid capture",
           this->srx882_edge_count_, static_cast<unsigned>(this->srx882_capture_duration_us_ / 1000UL));
  ESP_LOGI(TAG, "SRX882 raw capture complete valid=%s edges=%u duration=%u us",
           YESNO(this->srx882_capture_valid_), this->srx882_edge_count_, this->srx882_capture_duration_us_);

  if (this->cc1101_configured_) {
    this->cc1101_configure_ook_async_rx_();
    this->cc1101_enter_rx_();
    if (this->gdo0_pin_ != nullptr) this->rx_reset_packet_(micros(), this->gdo0_pin_->digital_read());
    this->rx_last_capture_ms_ = millis();
    this->rx_enabled_ = true;
  }
}

void RFBridgeComponent::clear_srx882_capture() {
  this->srx882_capture_valid_ = false;
  this->srx882_edge_count_ = 0;
  this->srx882_initial_level_ = 0;
  this->srx882_capture_duration_us_ = 0;
  memset(this->srx882_edges_, 0, sizeof(this->srx882_edges_));
  memset(this->srx882_levels_, 0, sizeof(this->srx882_levels_));
  snprintf(this->srx882_summary_, sizeof(this->srx882_summary_), "No SRX882 capture");
  ESP_LOGI(TAG, "SRX882 raw capture cleared");
}

bool RFBridgeComponent::replay_srx882_capture_stx882(uint8_t repeats) {
  if (!this->srx882_capture_valid_) {
    ESP_LOGW(TAG, "SRX882-to-STX882 replay unavailable; no valid SRX882 capture");
    return false;
  }
  return this->transmit_raw_edge_capture_stx882_(this->srx882_edges_, this->srx882_levels_,
                                                 this->srx882_edge_count_, this->srx882_initial_level_,
                                                 repeats, "SRX882_CAPTURE");
}

std::string RFBridgeComponent::get_srx882_summary() const {
  return std::string(this->srx882_summary_);
}

void RFBridgeComponent::start_rf_recorder(uint16_t duration_ms) {
  if (duration_ms < 500) duration_ms = 500;
  if (duration_ms > 10000) duration_ms = 10000;
  snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "Armed for %u ms", duration_ms);
  ESP_LOGI(TAG, "RF RECORDER armed: search window=%u ms; press the OEM remote now", duration_ms);

  // First collect the complete SRX882 search window. The SRX882 chatters while
  // idle, so this buffer is only an acquisition buffer. We then extract the
  // best dense RF burst and retain only that burst for replay.
  this->capture_srx882_raw(duration_ms);

  const uint16_t total_edges = this->srx882_edge_count_;
  if (total_edges < RX_MIN_EDGES) {
    this->srx882_capture_valid_ = false;
    snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "No valid burst found");
    snprintf(this->srx882_summary_, sizeof(this->srx882_summary_), "No recording");
    ESP_LOGW(TAG, "RF RECORDER: acquisition had too few edges (%u)", total_edges);
    return;
  }

  // A gap this large separates unrelated SRX882 chatter/bursts. Outprize data
  // edges are normally ~0.5/1.5 ms, with a ~4.5 ms header component.
  static constexpr uint16_t BURST_SPLIT_GAP_US = 12000;
  static constexpr uint16_t MIN_BURST_EDGES = 40;
  static constexpr uint16_t MAX_BURST_EDGES = 420;

  uint16_t best_start = 0;
  uint16_t best_end = 0;
  int32_t best_score = -1000000;
  uint16_t segment_start = 0;

  auto score_segment = [&](uint16_t seg_start, uint16_t seg_end) -> int32_t {
    if (seg_end <= seg_start) return -1000000;
    const uint16_t count = seg_end - seg_start;
    if (count < MIN_BURST_EDGES || count > MAX_BURST_EDGES) return -1000000;

    uint16_t protocol_like = 0;
    uint16_t very_short = 0;
    uint16_t mid_noise = 0;
    uint32_t duration = 0;
    for (uint16_t i = seg_start; i < seg_end; i++) {
      const uint16_t d = this->srx882_edges_[i];
      if (i != seg_start) duration += d;  // exclude the acquisition lead-in gap
      if ((d >= 350 && d <= 700) || (d >= 1150 && d <= 1850) || (d >= 3500 && d <= 6500)) {
        protocol_like++;
      } else if (d < 100) {
        very_short++;
      } else if (d < 350) {
        mid_noise++;
      }
    }

    // Reward Outprize-like timing and useful edge density; strongly penalize
    // the 60-300 us idle chatter characteristic of the raw SRX882 output.
    int32_t score = static_cast<int32_t>(protocol_like) * 12;
    score += static_cast<int32_t>(count) * 2;
    score -= static_cast<int32_t>(very_short) * 14;
    score -= static_cast<int32_t>(mid_noise) * 5;
    if (duration >= 25000 && duration <= 500000) score += 150;
    if (count >= 60 && count <= 180) score += 120;
    return score;
  };

  for (uint16_t i = 1; i <= total_edges; i++) {
    const bool at_end = i == total_edges;
    const bool split = !at_end && this->srx882_edges_[i] >= BURST_SPLIT_GAP_US;
    if (at_end || split) {
      const int32_t score = score_segment(segment_start, i);
      if (score > best_score) {
        best_score = score;
        best_start = segment_start;
        best_end = i;
      }
      segment_start = i;
    }
  }

  if (best_score < 0 || best_end <= best_start) {
    this->srx882_capture_valid_ = false;
    snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "No clean burst found");
    snprintf(this->srx882_summary_, sizeof(this->srx882_summary_), "No recording | %u acquisition edges",
             total_edges);
    ESP_LOGW(TAG, "RF RECORDER: no clean burst found in %u acquisition edges (best_score=%ld)",
             total_edges, static_cast<long>(best_score));
    return;
  }

  const uint16_t selected_count = best_end - best_start;
  const uint8_t selected_initial = (best_start == 0) ? this->srx882_initial_level_
                                                     : this->srx882_levels_[best_start - 1];
  uint32_t selected_duration = 0;

  // Compact the chosen burst in place. Replace the acquisition lead-in with a
  // short deterministic idle interval; retain every subsequent edge exactly.
  for (uint16_t out = 0; out < selected_count; out++) {
    const uint16_t src = best_start + out;
    this->srx882_edges_[out] = (out == 0) ? 250 : this->srx882_edges_[src];
    this->srx882_levels_[out] = this->srx882_levels_[src];
    selected_duration += this->srx882_edges_[out];
  }
  for (uint16_t i = selected_count; i < total_edges; i++) {
    this->srx882_edges_[i] = 0;
    this->srx882_levels_[i] = 0;
  }

  const uint16_t discarded_before = best_start;
  const uint16_t discarded_after = total_edges - best_end;
  this->srx882_edge_count_ = selected_count;
  this->srx882_initial_level_ = selected_initial;
  this->srx882_capture_duration_us_ = selected_duration;
  this->srx882_capture_valid_ = selected_count >= MIN_BURST_EDGES;

  if (this->srx882_capture_valid_) {
    this->rf_recording_number_++;
    snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "Recorded burst #%u",
             static_cast<unsigned>(this->rf_recording_number_));
    snprintf(this->srx882_summary_, sizeof(this->srx882_summary_),
             "Burst #%u: %u edges / %u ms | rejected %u+%u",
             static_cast<unsigned>(this->rf_recording_number_), selected_count,
             static_cast<unsigned>(selected_duration / 1000UL), discarded_before, discarded_after);
    ESP_LOGI(TAG,
             "RF RECORDER success recording=%u burst_edges=%u duration=%u us start=%u end=%u rejected_before=%u rejected_after=%u score=%ld",
             static_cast<unsigned>(this->rf_recording_number_), selected_count,
             static_cast<unsigned>(selected_duration), best_start, best_end,
             discarded_before, discarded_after, static_cast<long>(best_score));
  } else {
    snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "No valid burst found");
    snprintf(this->srx882_summary_, sizeof(this->srx882_summary_), "No recording");
    ESP_LOGW(TAG, "RF RECORDER: extracted burst was not valid");
  }
}

void RFBridgeComponent::clear_rf_recording() {
  this->clear_srx882_capture();
  snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "Ready");
  ESP_LOGI(TAG, "RF RECORDER cleared");
}

bool RFBridgeComponent::replay_rf_recording(uint8_t repeats) {
  if (!this->srx882_capture_valid_) {
    snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "Replay blocked: no recording");
    ESP_LOGW(TAG, "RF RECORDER replay blocked; no valid recording");
    return false;
  }
  snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_), "Replaying #%u",
           static_cast<unsigned>(this->rf_recording_number_));
  ESP_LOGI(TAG, "RF RECORDER replay start recording=%u repeats=%u",
           static_cast<unsigned>(this->rf_recording_number_), repeats);
  const bool ok = this->replay_srx882_capture_stx882(repeats);
  snprintf(this->rf_recorder_status_, sizeof(this->rf_recorder_status_),
           ok ? "Recorded #%u - replay OK" : "Recorded #%u - replay failed",
           static_cast<unsigned>(this->rf_recording_number_));
  ESP_LOGI(TAG, "RF RECORDER replay complete recording=%u ok=%s",
           static_cast<unsigned>(this->rf_recording_number_), YESNO(ok));
  return ok;
}

std::string RFBridgeComponent::get_rf_recorder_status() const {
  return std::string(this->rf_recorder_status_);
}

std::string RFBridgeComponent::get_rf_recording_summary() const {
  if (!this->srx882_capture_valid_) return std::string("No recording");
  char summary[160];
  snprintf(summary, sizeof(summary), "#%u | %u edges | %u ms | initial=%u",
           static_cast<unsigned>(this->rf_recording_number_), this->srx882_edge_count_,
           static_cast<unsigned>(this->srx882_capture_duration_us_ / 1000UL),
           this->srx882_initial_level_);
  return std::string(summary);
}


bool RFBridgeComponent::outprize_template_bit_(uint64_t frame, uint8_t bit_index, TxFrameMode mode) const {
  const uint8_t source_bit = (mode == TxFrameMode::LSB_NORMAL || mode == TxFrameMode::LSB_INVERTED)
                                 ? bit_index
                                 : (OUTPRIZE_TX_BITS - 1 - bit_index);
  bool value = ((frame >> source_bit) & 0x01ULL) != 0;
  if (mode == TxFrameMode::MSB_INVERTED || mode == TxFrameMode::LSB_INVERTED) value = !value;
  return value;
}

void RFBridgeComponent::log_rf_recording_edges_() const {
  if (!this->srx882_capture_valid_) {
    ESP_LOGW(TAG, "OUTPRIZE_TEMPLATE_DUMP unavailable; no RF recording");
    return;
  }
  ESP_LOGI(TAG, "===== OUTPRIZE_ACCEPTED_RECORDING =====");
  ESP_LOGI(TAG, "recording=%u edges=%u duration=%u us initial=%u",
           static_cast<unsigned>(this->rf_recording_number_), this->srx882_edge_count_,
           static_cast<unsigned>(this->srx882_capture_duration_us_), this->srx882_initial_level_);
  for (uint16_t row = 0; row < this->srx882_edge_count_; row += 12) {
    char line[360]{};
    size_t p = 0;
    p += snprintf(line + p, sizeof(line) - p, "  edges[%03u-%03u]", row,
                  static_cast<unsigned>(((row + 11) < this->srx882_edge_count_) ? (row + 11) : (this->srx882_edge_count_ - 1)));
    for (uint16_t i = row; i < this->srx882_edge_count_ && i < row + 12; i++) {
      p += snprintf(line + p, sizeof(line) - p, " %u:%u>%u", i, this->srx882_edges_[i],
                    this->srx882_levels_[i]);
    }
    ESP_LOGI(TAG, "%s", line);
  }
  ESP_LOGI(TAG, "=========================================");
}

bool RFBridgeComponent::analyze_rf_recording_outprize(uint32_t source_low24) {
  this->outprize_template_valid_ = false;
  snprintf(this->outprize_template_summary_, sizeof(this->outprize_template_summary_),
           "No valid Outprize template");
  if (!this->srx882_capture_valid_ || this->srx882_edge_count_ < OUTPRIZE_TX_BITS * 2) {
    ESP_LOGW(TAG, "OUTPRIZE_TEMPLATE analyze blocked; recording valid=%s edges=%u",
             YESNO(this->srx882_capture_valid_), this->srx882_edge_count_);
    return false;
  }

  this->log_rf_recording_edges_();
  const uint64_t source_frame = (static_cast<uint64_t>(this->outprize_remote_id_ & 0x7FF) << 24) |
                                (source_low24 & 0xFFFFFFULL);
  const TxFrameMode modes[] = {TxFrameMode::MSB_NORMAL, TxFrameMode::LSB_NORMAL,
                               TxFrameMode::MSB_INVERTED, TxFrameMode::LSB_INVERTED};
  int32_t best_score = -1000000;
  uint16_t best_start = 0;
  TxFrameMode best_mode = TxFrameMode::MSB_NORMAL;
  uint32_t best_pulse_sum = 0, best_short_sum = 0, best_long_sum = 0;
  uint16_t best_pulse_count = 0, best_short_count = 0, best_long_count = 0;

  const uint16_t required = OUTPRIZE_TX_BITS * 2;
  for (uint16_t start = 0; start + required <= this->srx882_edge_count_; start++) {
    for (TxFrameMode mode : modes) {
      int32_t score = 0;
      uint32_t pulse_sum = 0, short_sum = 0, long_sum = 0;
      uint16_t pulse_count = 0, short_count = 0, long_count = 0;
      for (uint8_t bit = 0; bit < OUTPRIZE_TX_BITS; bit++) {
        const uint16_t pulse = this->srx882_edges_[start + bit * 2];
        const uint16_t gap = this->srx882_edges_[start + bit * 2 + 1];
        const bool one = this->outprize_template_bit_(source_frame, bit, mode);
        const bool pulse_ok = pulse >= 350 && pulse <= 750;
        const bool gap_ok = one ? (gap >= 1050 && gap <= 1900) : (gap >= 300 && gap <= 800);
        score += pulse_ok ? 12 : -20;
        score += gap_ok ? 18 : -28;
        if (pulse_ok) { pulse_sum += pulse; pulse_count++; }
        if (gap_ok) {
          if (one) { long_sum += gap; long_count++; }
          else { short_sum += gap; short_count++; }
        }
      }
      // Favor an intact header/trailer around the 70 data edges rather than a clipped alignment.
      if (start >= 2) score += 20;
      if (start + required < this->srx882_edge_count_) score += 10;
      if (score > best_score) {
        best_score = score;
        best_start = start;
        best_mode = mode;
        best_pulse_sum = pulse_sum; best_short_sum = short_sum; best_long_sum = long_sum;
        best_pulse_count = pulse_count; best_short_count = short_count; best_long_count = long_count;
      }
    }
  }

  if (best_score < 700 || best_pulse_count < 28 || best_short_count == 0 || best_long_count == 0) {
    ESP_LOGW(TAG, "OUTPRIZE_TEMPLATE no credible 35-bit alignment score=%ld pulse=%u short=%u long=%u",
             static_cast<long>(best_score), best_pulse_count, best_short_count, best_long_count);
    snprintf(this->outprize_template_summary_, sizeof(this->outprize_template_summary_),
             "Analyze failed | score=%ld", static_cast<long>(best_score));
    return false;
  }

  this->outprize_template_data_start_ = best_start;
  this->outprize_template_mode_ = best_mode;
  this->outprize_template_pulse_us_ = best_pulse_sum / best_pulse_count;
  this->outprize_template_short_gap_us_ = best_short_sum / best_short_count;
  this->outprize_template_long_gap_us_ = best_long_sum / best_long_count;
  this->outprize_template_source_low24_ = source_low24 & 0xFFFFFFUL;
  this->outprize_template_score_ = static_cast<uint16_t>(best_score > 65535 ? 65535 : best_score);
  this->outprize_template_valid_ = true;

  const char *mode_name = "MSB_NORMAL";
  if (best_mode == TxFrameMode::LSB_NORMAL) mode_name = "LSB_NORMAL";
  else if (best_mode == TxFrameMode::MSB_INVERTED) mode_name = "MSB_INVERTED";
  else if (best_mode == TxFrameMode::LSB_INVERTED) mode_name = "LSB_INVERTED";
  snprintf(this->outprize_template_summary_, sizeof(this->outprize_template_summary_),
           "Ready | start=%u mode=%s pulse=%u short=%u long=%u score=%u",
           best_start, mode_name, this->outprize_template_pulse_us_,
           this->outprize_template_short_gap_us_, this->outprize_template_long_gap_us_,
           this->outprize_template_score_);
  ESP_LOGI(TAG, "===== OUTPRIZE_TEMPLATE_ANALYSIS =====");
  ESP_LOGI(TAG, "source full35=0x%09llX low24=0x%06X recording_edges=%u",
           static_cast<unsigned long long>(source_frame), source_low24 & 0xFFFFFFUL,
           this->srx882_edge_count_);
  ESP_LOGI(TAG, "alignment start=%u data_edges=%u header_edges=%u trailer_edges=%u mode=%s score=%ld",
           best_start, required, best_start, this->srx882_edge_count_ - best_start - required,
           mode_name, static_cast<long>(best_score));
  ESP_LOGI(TAG, "timing mean pulse=%u us short_gap=%u us long_gap=%u us counts[pulse=%u short=%u long=%u]",
           this->outprize_template_pulse_us_, this->outprize_template_short_gap_us_,
           this->outprize_template_long_gap_us_, best_pulse_count, best_short_count, best_long_count);
  for (uint8_t bit = 0; bit < OUTPRIZE_TX_BITS; bit++) {
    const bool one = this->outprize_template_bit_(source_frame, bit, best_mode);
    const uint16_t pulse = this->srx882_edges_[best_start + bit * 2];
    const uint16_t gap = this->srx882_edges_[best_start + bit * 2 + 1];
    ESP_LOGI(TAG, "  bit[%02u]=%u pulse=%u gap=%u class=%s", bit, one ? 1 : 0,
             pulse, gap, one ? "LONG" : "SHORT");
  }
  ESP_LOGI(TAG, "=====================================");
  return true;
}

bool RFBridgeComponent::replay_manufactured_outprize_low24(uint32_t low24, uint8_t repeats) {
  return this->replay_manufactured_outprize_low24(this->outprize_remote_id_, low24, repeats);
}

bool RFBridgeComponent::replay_manufactured_outprize_low24(uint32_t remote_id, uint32_t low24, uint8_t repeats) {
  // v1.3.28 canonical waveform learned from the accepted SRX882/STX882 captures.
  // This is intentionally independent of recorder RAM and survives reboot/firmware updates.
  // Envelope: 8 fixed header edges followed by 35 MSB-first PWM symbols.
  static const uint16_t header_edges[8] = {
      250, 1169, 4092, 1107, 2486, 363, 9030, 4506,
  };
  static constexpr uint16_t PULSE_US = 500;
  static constexpr uint16_t SHORT_GAP_US = 500;
  static constexpr uint16_t LONG_GAP_US = 1500;
  static constexpr uint16_t EDGE_COUNT = 8 + OUTPRIZE_TX_BITS * 2;

  const uint32_t prefix = remote_id & 0x7FF;
  const uint64_t frame = (static_cast<uint64_t>(prefix) << 24) |
                         (low24 & 0xFFFFFFULL);
  uint16_t edges[EDGE_COUNT]{};
  uint8_t levels[EDGE_COUNT]{};

  for (uint8_t i = 0; i < 8; i++) edges[i] = header_edges[i];
  for (uint8_t bit = 0; bit < OUTPRIZE_TX_BITS; bit++) {
    const uint16_t index = 8 + bit * 2;
    edges[index] = PULSE_US;
    const bool one = this->outprize_template_bit_(frame, bit, TxFrameMode::MSB_NORMAL);
    edges[index + 1] = one ? LONG_GAP_US : SHORT_GAP_US;
  }

  // Initial level is low. Every stored edge toggles the discrete OOK DATA output.
  bool level = false;
  uint32_t duration = 0;
  for (uint16_t i = 0; i < EDGE_COUNT; i++) {
    duration += edges[i];
    level = !level;
    levels[i] = level ? 1 : 0;
  }

  ESP_LOGI(TAG,
           "OUTPRIZE_BUILTIN TX full35=0x%09llX low24=0x%06X edges=%u duration=%u us "
           "timing[pulse=%u short=%u long=%u] repeats=%u",
           static_cast<unsigned long long>(frame), low24 & 0xFFFFFFUL, EDGE_COUNT,
           static_cast<unsigned>(duration), PULSE_US, SHORT_GAP_US, LONG_GAP_US,
           repeats == 0 ? 1 : repeats);
  return this->transmit_raw_edge_capture_stx882_(edges, levels, EDGE_COUNT, 0, repeats,
                                                  "OUTPRIZE_BUILTIN");
}

bool RFBridgeComponent::replay_manufactured_outprize(uint8_t speed_percent, OutprizeDirection direction,
                                                      bool rain_enabled, OutprizeVentCommand vent_command,
                                                      uint8_t repeats) {
  return this->replay_manufactured_outprize_low24(
      this->encode_outprize_low24(speed_percent, direction, rain_enabled, vent_command), repeats);
}

std::string RFBridgeComponent::get_outprize_template_summary() const {
  return std::string(this->outprize_template_summary_);
}


uint8_t RFBridgeComponent::decode_outprize_speed_(uint32_t low24) const {
  return this->outprize_codec_.decode_speed(low24);
}

void RFBridgeComponent::update_outprize_state_from_low24_(uint32_t low24, OutprizeCommandSource source) {
  low24 &= 0xFFFFFFUL;
  this->outprize_state_.valid = true;
  this->outprize_state_.low24 = low24;
  this->outprize_state_.powered = (low24 != 0x600000UL);
  this->outprize_state_.speed_percent = this->decode_outprize_speed_(low24);
  this->outprize_state_.direction = (low24 & 0x20) ? OutprizeDirection::IN : OutprizeDirection::OUT;
  this->outprize_state_.rain_enabled = (low24 & 0x10) != 0;
  this->outprize_state_.vent_command = static_cast<OutprizeVentCommand>(low24 & 0x0C);
  this->outprize_state_.source = source;
  this->outprize_state_.revision++;
  ESP_LOGI(TAG, "OUTPRIZE_STATE source=%s revision=%u power=%s speed=%u direction=%s rain=%s vent=0x%02X low24=0x%06X",
           this->get_outprize_command_source().c_str(), this->outprize_state_.revision,
           YESNO(this->outprize_state_.powered), this->outprize_state_.speed_percent,
           this->outprize_state_.direction == OutprizeDirection::IN ? "IN" : "OUT",
           YESNO(this->outprize_state_.rain_enabled), static_cast<uint8_t>(this->outprize_state_.vent_command), low24);
}

bool RFBridgeComponent::transmit_cached_state_(uint8_t repeats) {
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  this->suppress_oem_until_ms_ = millis() + 300;
  return this->replay_manufactured_outprize_low24(this->outprize_state_.low24, repeats);
}

bool RFBridgeComponent::send_outprize_complete_state(uint32_t remote_id, bool powered, uint8_t speed_percent,
                                                        OutprizeDirection direction, bool rain_enabled,
                                                        OutprizeVentCommand vent_command, uint8_t repeats) {
  const uint8_t normalized_speed =
      static_cast<uint8_t>(std::min<uint16_t>(100, ((speed_percent + 5) / 10) * 10));
  const uint32_t low24 = powered
      ? this->encode_outprize_low24(normalized_speed, direction, rain_enabled, vent_command)
      : 0x600000UL;

  ESP_LOGI(TAG,
           "OUTPRIZE_API addressed remote=0x%03X powered=%s speed_requested=%u speed_encoded=%u "
           "direction=%s rain=%s vent=0x%02X -> low24=0x%06X",
           remote_id & 0x7FF, YESNO(powered), speed_percent, normalized_speed,
           direction == OutprizeDirection::IN ? "IN" : "OUT", YESNO(rain_enabled),
           static_cast<uint8_t>(vent_command), low24);

  const uint32_t prefix = remote_id & 0x7FF;
  ESP_LOGI(TAG,
           "OUTPRIZE_API addressed route codec=%s tx_backend=%s remote=0x%03X full35=0x%09llX",
           this->outprize_codec_.id(), this->outprize_codec_.tx_backend(), prefix,
           static_cast<unsigned long long>((static_cast<uint64_t>(prefix) << 24) | (low24 & 0xFFFFFFULL)));

  this->suppress_oem_until_ms_ = millis() + 300;
  return this->replay_manufactured_outprize_low24(prefix, low24, repeats);
}

bool RFBridgeComponent::set_outprize_complete_state(bool powered, uint8_t speed_percent, OutprizeDirection direction,
                                                     bool rain_enabled, OutprizeVentCommand vent_command, uint8_t repeats) {
  // POWER OFF is the dedicated 0x600000 command.  For an awake/on state,
  // manufacture the complete state packet.  The vent nibble is a transient
  // action: CLOSE (0x04), OPEN (0x08), STOP (0x0C), or NONE (0x00).
  const uint8_t normalized_speed = static_cast<uint8_t>(std::min<uint16_t>(100, ((speed_percent + 5) / 10) * 10));
  const uint32_t low24 = powered
      ? this->encode_outprize_low24(normalized_speed, direction, rain_enabled, vent_command)
      : 0x600000UL;

  ESP_LOGI(TAG,
           "OUTPRIZE_API complete powered=%s speed_requested=%u speed_encoded=%u direction=%s rain=%s "
           "vent=0x%02X -> low24=0x%06X",
           YESNO(powered), speed_percent, normalized_speed,
           direction == OutprizeDirection::IN ? "IN" : "OUT", YESNO(rain_enabled),
           static_cast<uint8_t>(vent_command), low24);

  if (powered && (vent_command == OutprizeVentCommand::CLOSE || vent_command == OutprizeVentCommand::STOP)) {
    ESP_LOGW(TAG,
             "OUTPRIZE_API powered=true with vent command 0x%02X: this is a vent action and may stop/close the fan. "
             "Use vent_command=0 for a fan-speed-only command or 8 to open the vent.",
             static_cast<uint8_t>(vent_command));
  }

  this->update_outprize_state_from_low24_(low24, OutprizeCommandSource::HOME_ASSISTANT);
  return this->transmit_cached_state_(repeats);
}

bool RFBridgeComponent::set_outprize_power(bool powered, uint8_t repeats) {
  if (!powered) return this->set_outprize_complete_state(false, 0, OutprizeDirection::OUT, false, OutprizeVentCommand::NONE, repeats);
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  this->outprize_state_.powered = true;
  this->outprize_state_.low24 = this->encode_outprize_low24(this->outprize_state_.speed_percent, this->outprize_state_.direction,
                                                            this->outprize_state_.rain_enabled, this->outprize_state_.vent_command);
  this->outprize_state_.source = OutprizeCommandSource::HOME_ASSISTANT;
  this->outprize_state_.revision++;
  return this->transmit_cached_state_(repeats);
}

bool RFBridgeComponent::set_outprize_speed(uint8_t speed_percent, uint8_t repeats) {
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  return this->set_outprize_complete_state(true, speed_percent, this->outprize_state_.direction,
                                            this->outprize_state_.rain_enabled, this->outprize_state_.vent_command, repeats);
}

bool RFBridgeComponent::set_outprize_direction(OutprizeDirection direction, uint8_t repeats) {
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  return this->set_outprize_complete_state(this->outprize_state_.powered, this->outprize_state_.speed_percent, direction,
                                            this->outprize_state_.rain_enabled, this->outprize_state_.vent_command, repeats);
}

bool RFBridgeComponent::set_outprize_rain(bool enabled, uint8_t repeats) {
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  return this->set_outprize_complete_state(this->outprize_state_.powered, this->outprize_state_.speed_percent,
                                            this->outprize_state_.direction, enabled, this->outprize_state_.vent_command, repeats);
}

bool RFBridgeComponent::set_outprize_vent(OutprizeVentCommand command, uint8_t repeats) {
  if (!this->outprize_state_.valid) this->update_outprize_state_from_low24_(0x600040, OutprizeCommandSource::HOME_ASSISTANT);
  return this->set_outprize_complete_state(this->outprize_state_.powered, this->outprize_state_.speed_percent,
                                            this->outprize_state_.direction, this->outprize_state_.rain_enabled, command, repeats);
}

std::string RFBridgeComponent::get_outprize_command_source() const {
  switch (this->outprize_state_.source) {
    case OutprizeCommandSource::HOME_ASSISTANT: return "home_assistant";
    case OutprizeCommandSource::OEM_REMOTE: return "oem_remote";
    case OutprizeCommandSource::RESTORED: return "restored";
    default: return "unknown";
  }
}

std::string RFBridgeComponent::get_outprize_state_summary() const {
  if (!this->outprize_state_.valid) return "invalid";
  char buffer[192];
  snprintf(buffer, sizeof(buffer), "rev=%u source=%s power=%s speed=%u direction=%s rain=%s vent=0x%02X low24=0x%06X",
           this->outprize_state_.revision, this->get_outprize_command_source().c_str(), YESNO(this->outprize_state_.powered),
           this->outprize_state_.speed_percent, this->outprize_state_.direction == OutprizeDirection::IN ? "in" : "out",
           YESNO(this->outprize_state_.rain_enabled), static_cast<uint8_t>(this->outprize_state_.vent_command),
           this->outprize_state_.low24);
  return std::string(buffer);
}


std::string RFBridgeComponent::get_bridge_capabilities() const {
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "bridge=rfbridge version=%s radios[cc1101_rx=%s,cc1101_tx=yes,stx882_tx=%s,srx882_rx=%s] codecs[%s]",
           RFBRIDGE_VERSION, YESNO(this->cc1101_configured_), YESNO(this->stx882_data_pin_ != nullptr),
           YESNO(this->srx882_data_pin_ != nullptr), this->outprize_codec_.capability_summary().c_str());
  return buffer;
}

bool RFBridgeComponent::has_codec(const std::string &codec_id) const {
  return codec_id == this->outprize_codec_.id();
}

}  // namespace rfbridge
}  // namespace esphome
