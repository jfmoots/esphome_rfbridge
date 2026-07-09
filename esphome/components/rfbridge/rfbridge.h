#pragma once

#include <cstdint>
#include <cstring>

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

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
  void set_outprize_remote_id(uint32_t remote_id) { this->outprize_remote_id_ = remote_id & 0x7FF; }

  uint32_t encode_outprize_low24(uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                                 OutprizeVentCommand vent_command) const;

  bool send_outprize(uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                     OutprizeVentCommand vent_command);
  bool send_outprize(uint32_t remote_id, uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                     OutprizeVentCommand vent_command);
  bool send_outprize_low24(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_low24(uint32_t remote_id, uint32_t low24, uint8_t repeats);
  bool send_outprize_power_off(uint8_t repeats = 8) { return this->send_outprize_low24(0x600000, repeats); }
  bool send_outprize_fan_off(uint8_t repeats = 8) { return this->send_outprize_low24(0x600040, repeats); }

  // Reverse-engineering test helpers. These are intentionally public so they
  // can be called from ESPHome lambda buttons during protocol reconstruction.
  bool send_outprize_power_off_lsb(uint8_t repeats = 8);
  bool send_outprize_power_off_inv(uint8_t repeats = 8);
  bool send_outprize_power_off_inv_lsb(uint8_t repeats = 8);
  bool send_outprize_raw_oem_power_off(uint8_t repeats = 8);

  // v1.3.12 frequency-trim test helpers for Power Off.
  bool send_outprize_power_off_433900(uint8_t repeats = 8);
  bool send_outprize_power_off_433920(uint8_t repeats = 8);
  bool send_outprize_power_off_433940(uint8_t repeats = 8);
  bool send_outprize_power_off_433950(uint8_t repeats = 8);
  bool send_outprize_power_off_433970(uint8_t repeats = 8);

  // v1.3.18 RF-mode characterization helpers. These keep the learned/proven
  // Outprize Power Off payload unchanged and vary only radio-layer TX settings.
  bool send_outprize_power_off_rf_default(uint8_t repeats = 8);
  bool send_outprize_power_off_rf_inverted_ook(uint8_t repeats = 8);
  bool send_outprize_power_off_rf_pa_80(uint8_t repeats = 8);
  bool send_outprize_power_off_rf_pa_60(uint8_t repeats = 8);
  bool send_outprize_power_off_rf_frend0_10(uint8_t repeats = 8);
  bool send_outprize_power_off_rf_mdmcfg2_33(uint8_t repeats = 8);

  bool send_outprize_low24_lsb(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_low24_inverted(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_low24_lsb_inverted(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_raw_full35(uint64_t full35, uint8_t repeats = 8);
  bool send_outprize_raw_full35_lsb(uint64_t full35, uint8_t repeats = 8);
  bool replay_last_capture(uint8_t repeats = 1);
  bool replay_last_outprize_learned(uint8_t repeats = 8);
  void clear_last_outprize_learned();
  void compare_last_outprize_learned();
  bool send_ook_test_burst(uint16_t pulse_us = 500, uint16_t pulse_count = 240, uint8_t repeats = 8);
  bool send_ook_carrier_test(uint16_t duration_ms = 500);

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
  void rx_log_full_capture_timeline_(uint32_t capture_no);
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
    uint64_t full_packet{0};
    uint16_t remote_id{0};
  };

  bool rx_outprize_decode_from_index_(uint16_t start_index, OutprizeDecodeCandidate *candidate) const;
  uint16_t rx_score_outprize_candidate_(OutprizeDecodeCandidate *candidate) const;
  uint16_t rx_normalize_pulse_(uint16_t pulse_us) const;
  uint32_t rx_capture_fingerprint_() const;
  void rx_reset_packet_(uint32_t now_us, bool level);

  static constexpr uint16_t RX_MAX_EDGES = 360;
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
  // Keep the old single-frame decoder, but allow full RSSI-window captures
  // containing multiple repeats to be scanned for a valid 35-bit frame.
  static constexpr uint16_t OUTPRIZE_MAX_EDGES = RX_MAX_EDGES;
  static constexpr uint16_t OUTPRIZE_SHORT_US_MIN = 350;
  static constexpr uint16_t OUTPRIZE_SHORT_US_MAX = 750;
  static constexpr uint16_t OUTPRIZE_LONG_US_MIN = 1150;
  static constexpr uint16_t OUTPRIZE_LONG_US_MAX = 1800;
  static constexpr uint16_t OUTPRIZE_SYNC_US_MIN = 3800;
  static constexpr uint16_t OUTPRIZE_SYNC_US_MAX = 5200;

  bool diagnostic_logging_{false};
  uint8_t tx_freq2_{0x10};
  uint8_t tx_freq1_{0xB0};
  uint8_t tx_freq0_{0x71};
  const char *tx_freq_label_{"433.920 MHz"};
  uint32_t outprize_remote_id_{OUTPRIZE_DEFAULT_PREFIX};
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
  uint8_t rx_capture_initial_level_{0};
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

  bool outprize_learned_valid_{false};
  uint64_t outprize_learned_full35_{0};
  uint32_t outprize_learned_low24_{0};
  uint16_t outprize_learned_remote_id_{0};
  uint16_t outprize_learned_bits_{0};
  uint32_t outprize_learned_capture_no_{0};
  uint16_t outprize_learned_score_{0};
  uint16_t outprize_learned_start_index_{0};
  uint16_t outprize_learned_stop_index_{0};
  uint16_t outprize_learned_edge_count_{0};
  uint16_t outprize_learned_edges_[RX_MAX_EDGES]{};
  char outprize_learned_binary_[72]{};

  uint32_t outprize_speed_base_(uint8_t speed_percent) const;
  void cc1101_configure_ook_async_tx_();
  void cc1101_set_tx_frequency_(uint8_t freq2, uint8_t freq1, uint8_t freq0, const char *label);
  bool transmit_power_off_with_frequency_(uint8_t freq2, uint8_t freq1, uint8_t freq0, const char *label, uint8_t repeats);
  bool transmit_power_off_with_rf_profile_(const char *label, bool inverted_ook, uint8_t pa, uint8_t frend0, uint8_t mdmcfg2, uint8_t repeats);
  bool cc1101_calibrate_for_tx_();
  void tx_write_data_(bool level);
  void tx_write_carrier_(bool on);
  void tx_log_marcstate_(const char *stage);
  void tx_dump_status_(const char *stage);
  enum class TxFrameMode : uint8_t { MSB_NORMAL = 0, LSB_NORMAL = 1, MSB_INVERTED = 2, LSB_INVERTED = 3 };
  void tx_log_frame_bits_(uint64_t frame, uint8_t bits, TxFrameMode mode, const char *label) const;
  void tx_send_outprize_frame_(uint32_t prefix, uint32_t low24);
  void tx_send_frame_bits_(uint64_t frame, uint8_t bits, TxFrameMode mode);
  bool transmit_low24_(uint32_t remote_id, uint32_t low24, uint8_t repeats = 3);
  bool transmit_low24_mode_(uint32_t remote_id, uint32_t low24, uint8_t repeats, TxFrameMode mode, const char *label);
  bool transmit_full35_mode_(uint64_t full35, uint8_t repeats, TxFrameMode mode, const char *label);
  bool transmit_last_capture_(uint8_t repeats = 1);
  bool transmit_learned_outprize_(uint8_t repeats = 8);
  uint16_t tx_build_edge_deltas_(uint64_t frame, uint8_t bits, TxFrameMode mode, bool include_preamble, uint16_t *out, uint16_t max_edges) const;
  void log_outprize_edge_compare_(uint64_t frame, uint8_t bits, TxFrameMode mode) const;
  bool transmit_ook_test_burst_(uint16_t pulse_us, uint16_t pulse_count, uint8_t repeats);
  bool start_ook_carrier_test_(uint16_t duration_ms);
  void finish_ook_carrier_test_();
  void tx_carrier_loop_();

  static constexpr uint16_t OUTPRIZE_TX_HEADER_ON_US = 4596;
  static constexpr uint16_t OUTPRIZE_TX_HEADER_OFF_US = 4513;
  // Legacy synthetic header constants retained for compatibility with older diagnostics.
  static constexpr uint16_t OUTPRIZE_TX_RESET_GAP_US = OUTPRIZE_TX_HEADER_ON_US;
  static constexpr uint16_t OUTPRIZE_TX_SYNC_US = OUTPRIZE_TX_HEADER_OFF_US;
  static constexpr uint16_t OUTPRIZE_TX_PULSE_US = 488;
  static constexpr uint16_t OUTPRIZE_TX_ZERO_GAP_US = 488;
  static constexpr uint16_t OUTPRIZE_TX_ONE_GAP_US = 1464;
  static constexpr uint16_t OUTPRIZE_TX_INTER_FRAME_GAP_US = 2460;
  static constexpr uint16_t OUTPRIZE_TX_BITS = 35;
  static constexpr uint32_t OUTPRIZE_DEFAULT_PREFIX = 0x6CF;
  static constexpr uint8_t CC1101_TX_PA_TEST = 0xC0;

  bool tx_ook_inverted_{false};
  uint8_t tx_pa_value_{CC1101_TX_PA_TEST};
  uint8_t tx_frend0_value_{0x11};
  uint8_t tx_mdmcfg2_value_{0x30};

  bool tx_carrier_active_{false};
  uint32_t tx_carrier_started_ms_{0};
  uint16_t tx_carrier_duration_ms_{0};

};


template<typename... Ts> class SendOutprizeLow24Action : public Action<Ts...>, public Parented<RFBridgeComponent> {
 public:
  TEMPLATABLE_VALUE(uint32_t, remote_id)
  TEMPLATABLE_VALUE(uint32_t, low24)
  TEMPLATABLE_VALUE(uint8_t, repeats)

  void play(Ts... x) override {
    const uint32_t remote_id = this->remote_id_.value(x...);
    const uint32_t low24 = this->low24_.value(x...);
    const uint8_t repeats = this->repeats_.value(x...);
    this->parent_->send_outprize_low24(remote_id, low24, repeats);
  }
};

template<typename... Ts> class SendOutprizePowerOffAction : public Action<Ts...>, public Parented<RFBridgeComponent> {
 public:
  TEMPLATABLE_VALUE(uint32_t, remote_id)
  TEMPLATABLE_VALUE(uint8_t, repeats)

  void play(Ts... x) override {
    const uint32_t remote_id = this->remote_id_.value(x...);
    const uint8_t repeats = this->repeats_.value(x...);
    this->parent_->send_outprize_low24(remote_id, 0x600000, repeats);
  }
};

template<typename... Ts> class SendOutprizeFanOffAction : public Action<Ts...>, public Parented<RFBridgeComponent> {
 public:
  TEMPLATABLE_VALUE(uint32_t, remote_id)
  TEMPLATABLE_VALUE(uint8_t, repeats)

  void play(Ts... x) override {
    const uint32_t remote_id = this->remote_id_.value(x...);
    const uint8_t repeats = this->repeats_.value(x...);
    this->parent_->send_outprize_low24(remote_id, 0x600040, repeats);
  }
};

}  // namespace rfbridge
}  // namespace esphome
