#pragma once

#include <cstdint>

#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace rfbridge {

enum class OutprizeDirection : uint8_t {
  OUT = 0,
  IN = 1,
};

enum class OutprizeVentCommand : uint8_t {
  NONE = 0x00,
  CLOSE = 0x04,
  OPEN = 0x08,
  STOP = 0x0C,
};

class RFBridgeComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_cs_pin(GPIOPin *pin) { this->cs_pin_ = pin; }
  void set_sck_pin(GPIOPin *pin) { this->sck_pin_ = pin; }
  void set_mosi_pin(GPIOPin *pin) { this->mosi_pin_ = pin; }
  void set_miso_pin(GPIOPin *pin) { this->miso_pin_ = pin; }
  void set_gdo0_pin(GPIOPin *pin) { this->gdo0_pin_ = pin; }
  void set_gdo2_pin(GPIOPin *pin) { this->gdo2_pin_ = pin; }
  void set_diagnostic_logging(bool diagnostic_logging) { this->diagnostic_logging_ = diagnostic_logging; }

  uint32_t encode_outprize_low24(uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                                 OutprizeVentCommand vent_command) const;

  bool send_outprize(uint32_t remote_id, uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                     OutprizeVentCommand vent_command);

 protected:
  GPIOPin *cs_pin_{nullptr};
  GPIOPin *sck_pin_{nullptr};
  GPIOPin *mosi_pin_{nullptr};
  GPIOPin *miso_pin_{nullptr};
  GPIOPin *gdo0_pin_{nullptr};
  GPIOPin *gdo2_pin_{nullptr};

  bool cc1101_begin_();
  bool cc1101_detected_{false};
  bool cc1101_configured_{false};
  uint8_t cc1101_partnum_{0xFF};
  uint8_t cc1101_version_{0xFF};
  void cc1101_reset_();
  void cc1101_configure_ook_async_rx_();
  void cc1101_dump_registers_(const char *stage);
  void cc1101_log_register_(const char *name, uint8_t addr, int expected = -1);
  void cc1101_enter_rx_();
  void cc1101_enter_idle_();

  void cc1101_write_reg_(uint8_t addr, uint8_t value);
  uint8_t cc1101_read_reg_(uint8_t addr);
  uint8_t cc1101_read_status_(uint8_t addr);
  uint8_t cc1101_strobe_(uint8_t strobe);
  void cc1101_write_patable_(uint8_t value);
  int16_t cc1101_read_rssi_dbm_();

  void spi_select_();
  void spi_deselect_();
  uint8_t spi_transfer_byte_(uint8_t value);
  void spi_write_byte_(uint8_t value) { (void) this->spi_transfer_byte_(value); }
  uint8_t spi_read_byte_() { return this->spi_transfer_byte_(0x00); }

  void rx_setup_();
  void rx_poll_();
  void rx_capture_window_(int16_t trigger_rssi_dbm);
  void rx_finish_capture_(uint32_t start_us, uint32_t end_us, int16_t trigger_rssi_dbm);
  void rx_log_raw_timings_(uint32_t capture_no);
  void rx_log_pulse_histogram_(uint32_t capture_no);
  void rx_log_protocol_analysis_(uint32_t capture_no);
  bool rx_log_outprize_decode_(uint32_t capture_no);
  struct OutprizeDecodeCandidate {
    bool valid{false};
    bool bits[64]{};
    uint16_t bit_count{0};
    uint16_t start_index{0};
    uint16_t stop_index{0};
    uint16_t invalid_count{0};
    uint16_t score{0};
    uint32_t low24{0};
  };

  bool rx_outprize_decode_from_index_(uint16_t start_index, OutprizeDecodeCandidate *candidate) const;
  uint16_t rx_score_outprize_candidate_(OutprizeDecodeCandidate *candidate) const;
  uint16_t rx_normalize_pulse_(uint16_t pulse_us) const;
  uint32_t rx_capture_fingerprint_() const;
  void rx_reset_packet_(uint32_t now_us, bool level);

  static constexpr uint16_t RX_MAX_EDGES = 220;
  // Match the original diagnostic sniffer shape: ignore GDO0 chatter until
  // RSSI indicates a real nearby transmitter, then capture a fixed window.
  static constexpr int16_t RX_RSSI_ARM_DBM = -80;
  static constexpr uint32_t RX_CAPTURE_WINDOW_US = 140000;
  static constexpr uint32_t RX_CAPTURE_COOLDOWN_MS = 250;
  static constexpr uint32_t RX_RSSI_POLL_INTERVAL_MS = 10;
  static constexpr uint32_t RX_MIN_PACKET_US = 15000;
  static constexpr uint16_t RX_MIN_EDGES = 40;
  static constexpr uint16_t RX_HIST_BIN_US = 64;
  static constexpr uint16_t RX_HIST_BIN_COUNT = 80;
  static constexpr uint16_t RX_ANALYSIS_MIN_US = 50;
  static constexpr uint16_t RX_ANALYSIS_MAX_US = 2000;
  static constexpr uint16_t RX_ANALYSIS_BIN_US = 64;
  static constexpr uint16_t RX_ANALYSIS_BIN_COUNT = 40;
  static constexpr uint16_t RX_ANALYSIS_MAX_PRINTED_SYMBOLS = 96;
  static constexpr uint16_t OUTPRIZE_MIN_EDGES = 60;
  static constexpr uint16_t OUTPRIZE_MAX_EDGES = 95;
  static constexpr uint16_t OUTPRIZE_SHORT_US_MIN = 350;
  static constexpr uint16_t OUTPRIZE_SHORT_US_MAX = 750;
  static constexpr uint16_t OUTPRIZE_LONG_US_MIN = 1150;
  static constexpr uint16_t OUTPRIZE_LONG_US_MAX = 1800;
  static constexpr uint16_t OUTPRIZE_SYNC_US_MIN = 3800;
  static constexpr uint16_t OUTPRIZE_SYNC_US_MAX = 5200;

  bool diagnostic_logging_{false};
  bool rx_last_outprize_like_{false};
  bool rx_enabled_{false};
  bool rx_have_level_{false};
  bool rx_last_level_{false};
  uint32_t rx_last_edge_us_{0};
  uint32_t rx_packet_start_us_{0};
  uint32_t rx_last_activity_log_ms_{0};
  uint32_t rx_last_rssi_poll_ms_{0};
  uint32_t rx_last_capture_ms_{0};
  uint16_t rx_edge_count_{0};
  uint16_t rx_overruns_{0};
  uint32_t rx_packets_seen_{0};
  uint32_t rx_edges_seen_{0};
  uint32_t rx_last_packet_duration_us_{0};
  uint16_t rx_last_packet_edges_{0};
  uint16_t rx_last_min_gap_us_{0};
  uint16_t rx_last_max_gap_us_{0};
  uint16_t rx_last_avg_gap_us_{0};
  uint16_t rx_discarded_partials_{0};
  int16_t rx_last_rssi_dbm_{0};
  int16_t rx_last_trigger_rssi_dbm_{0};
  uint32_t rx_last_fingerprint_{0};
  uint16_t rx_last_filtered_edges_{0};
  uint16_t rx_last_unique_bins_{0};
  uint16_t rx_edges_[RX_MAX_EDGES]{};
  uint8_t rx_levels_[RX_MAX_EDGES]{};

  uint8_t outprize_speed_code_(uint8_t speed_percent) const;
  bool transmit_low24_(uint32_t remote_id, uint32_t low24);
};

}  // namespace rfbridge
}  // namespace esphome
