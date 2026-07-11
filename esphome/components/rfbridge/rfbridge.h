#pragma once

#include <cstdint>
#include <cstring>
#include <string>

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

enum class OutprizeCommandSource : uint8_t { UNKNOWN = 0, HOME_ASSISTANT = 1, OEM_REMOTE = 2, RESTORED = 3 };

struct OutprizeState {
  bool valid{false};
  bool powered{false};
  uint8_t speed_percent{0};
  OutprizeDirection direction{OutprizeDirection::OUT};
  bool rain_enabled{false};
  OutprizeVentCommand vent_command{OutprizeVentCommand::NONE};
  uint32_t low24{0x600000};
  uint32_t revision{0};
  OutprizeCommandSource source{OutprizeCommandSource::UNKNOWN};
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
  void set_stx882_data_pin(GPIOPin *pin) { this->stx882_data_pin_ = pin; }
  void set_srx882_data_pin(GPIOPin *pin) { this->srx882_data_pin_ = pin; }
  void set_srx882_enable_pin(GPIOPin *pin) { this->srx882_enable_pin_ = pin; }
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
  bool send_outprize_power_off_profile(uint8_t profile, uint8_t repeats = 8);

  // v1.3.22 soft ASK / softened-envelope experiments. These keep the
  // known Power Off payload unchanged and vary only CC1101 TX envelope/profile settings.
  bool send_outprize_power_off_soft_ask_profile(uint8_t profile, uint8_t repeats = 8);

  bool send_outprize_low24_lsb(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_low24_inverted(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_low24_lsb_inverted(uint32_t low24, uint8_t repeats = 8);
  bool send_outprize_raw_full35(uint64_t full35, uint8_t repeats = 8);
  bool send_outprize_raw_full35_lsb(uint64_t full35, uint8_t repeats = 8);
  bool replay_last_capture(uint8_t repeats = 1);
  bool replay_last_outprize_learned(uint8_t repeats = 8);
  void clear_last_outprize_learned();
  void compare_last_outprize_learned();

  // v1.3.20: deterministic known-frame helpers so testing is never blocked
  // by RSSI-gated capture/learning flakiness.
  void set_outprize_learned_frame(uint64_t full35 = ((static_cast<uint64_t>(OUTPRIZE_DEFAULT_PREFIX) << 24) | 0x600000ULL));
  bool replay_known_outprize_power_off(uint8_t repeats = 8);
  void compare_known_outprize_power_off();

  // v1.3.21: raw OEM edge replay. These bypass the 35-bit encoder and replay
  // the captured edge timing array directly through the CC1101 async TX input.
  bool replay_last_outprize_raw_capture(uint8_t repeats = 1);
  bool replay_last_raw_capture(uint8_t repeats = 1) { return this->replay_last_capture(repeats); }
  bool send_ook_test_burst(uint16_t pulse_us = 500, uint16_t pulse_count = 240, uint8_t repeats = 8);
  bool send_ook_carrier_test(uint16_t duration_ms = 500);

  // v1.3.23: explicit learning UX and multi-frame button capture.
  void start_rf_sequence_capture(uint16_t duration_ms = 1500);
  void clear_rf_sequence_capture();
  bool replay_rf_sequence_raw(uint8_t repeats = 1);
  bool has_learned_command() const { return this->outprize_learned_valid_; }
  bool has_sequence_capture() const { return this->sequence_count_ > 0; }
  std::string get_last_learned_summary() const;
  std::string get_sequence_summary() const;

  // v1.3.24: discrete ASK/OOK backends. The STX882 provides a direct
  // carrier-on/carrier-off transmitter path; the SRX882 provides an
  // explicit raw-capture receiver path independent of the CC1101 modem.
  bool replay_last_outprize_raw_capture_stx882(uint8_t repeats = 1);
  bool replay_rf_sequence_raw_stx882(uint8_t repeats = 1);
  bool send_outprize_power_off_stx882(uint8_t repeats = 8);
  void capture_srx882_raw(uint16_t duration_ms = 1500);
  void clear_srx882_capture();
  bool replay_srx882_capture_stx882(uint8_t repeats = 1);
  bool has_srx882_capture() const { return this->srx882_capture_valid_; }
  std::string get_srx882_summary() const;

  // v1.3.25: high-level RF recorder built on the proven SRX882 -> STX882
  // raw edge path. Recording is explicit and never overwritten by ambient RF.
  void start_rf_recorder(uint16_t duration_ms = 3000);
  void clear_rf_recording();
  bool replay_rf_recording(uint8_t repeats = 1);
  bool has_rf_recording() const { return this->srx882_capture_valid_; }
  std::string get_rf_recorder_status() const;
  std::string get_rf_recording_summary() const;

  // v1.3.27: use a known-good SRX882 recording as an RF-envelope template,
  // identify the embedded 35-bit Outprize payload, and manufacture new full-state
  // commands while preserving the accepted header, pulse widths, polarity, and trailer.
  bool analyze_rf_recording_outprize(uint32_t source_low24 = 0x600000);
  bool replay_manufactured_outprize_low24(uint32_t low24, uint8_t repeats = 1);
  bool replay_manufactured_outprize(uint8_t speed_percent, OutprizeDirection direction,
                                    bool rain_enabled, OutprizeVentCommand vent_command,
                                    uint8_t repeats = 1);
  bool has_outprize_template() const { return this->outprize_template_valid_; }
  std::string get_outprize_template_summary() const;

  // v1.4.x stable state/transport contract. No ESPHome fan/cover entities live here.
  bool set_outprize_complete_state(bool powered, uint8_t speed_percent, OutprizeDirection direction,
                                    bool rain_enabled, OutprizeVentCommand vent_command, uint8_t repeats = 1);
  bool set_outprize_power(bool powered, uint8_t repeats = 1);
  bool set_outprize_speed(uint8_t speed_percent, uint8_t repeats = 1);
  bool set_outprize_direction(OutprizeDirection direction, uint8_t repeats = 1);
  bool set_outprize_rain(bool enabled, uint8_t repeats = 1);
  bool set_outprize_vent(OutprizeVentCommand command, uint8_t repeats = 1);
  const OutprizeState &get_outprize_state() const { return this->outprize_state_; }
  bool has_outprize_state() const { return this->outprize_state_.valid; }
  bool get_outprize_power() const { return this->outprize_state_.powered; }
  uint8_t get_outprize_speed() const { return this->outprize_state_.speed_percent; }
  bool get_outprize_direction_in() const { return this->outprize_state_.direction == OutprizeDirection::IN; }
  bool get_outprize_rain() const { return this->outprize_state_.rain_enabled; }
  uint8_t get_outprize_vent_command() const { return static_cast<uint8_t>(this->outprize_state_.vent_command); }
  uint32_t get_outprize_low24() const { return this->outprize_state_.low24; }
  uint32_t get_outprize_revision() const { return this->outprize_state_.revision; }
  std::string get_outprize_state_summary() const;
  std::string get_outprize_command_source() const;

 protected:
  GPIOPin *cs_pin_{nullptr};
  GPIOPin *sck_pin_{nullptr};
  GPIOPin *mosi_pin_{nullptr};
  GPIOPin *miso_pin_{nullptr};
  GPIOPin *gdo0_pin_{nullptr};
  GPIOPin *gdo2_pin_{nullptr};
  GPIOPin *stx882_data_pin_{nullptr};
  GPIOPin *srx882_data_pin_{nullptr};
  GPIOPin *srx882_enable_pin_{nullptr};

  OutprizeState outprize_state_{};
  uint32_t suppress_oem_until_ms_{0};
  void update_outprize_state_from_low24_(uint32_t low24, OutprizeCommandSource source);
  uint8_t decode_outprize_speed_(uint32_t low24) const;
  bool transmit_cached_state_(uint8_t repeats);

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
  void sequence_store_current_capture_(uint32_t capture_no, bool decoded);
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
  uint8_t outprize_learned_initial_level_{0};
  uint16_t outprize_learned_edges_[RX_MAX_EDGES]{};
  uint8_t outprize_learned_levels_[RX_MAX_EDGES]{};
  char outprize_learned_binary_[72]{};

  static constexpr uint8_t RF_SEQUENCE_MAX_FRAMES = 8;
  bool sequence_active_{false};
  uint32_t sequence_started_ms_{0};
  uint32_t sequence_until_ms_{0};
  uint8_t sequence_count_{0};
  uint32_t sequence_capture_no_[RF_SEQUENCE_MAX_FRAMES]{};
  uint32_t sequence_frame_ms_[RF_SEQUENCE_MAX_FRAMES]{};
  uint16_t sequence_edge_count_[RF_SEQUENCE_MAX_FRAMES]{};
  uint8_t sequence_initial_level_[RF_SEQUENCE_MAX_FRAMES]{};
  bool sequence_decoded_[RF_SEQUENCE_MAX_FRAMES]{};
  uint64_t sequence_full35_[RF_SEQUENCE_MAX_FRAMES]{};
  uint32_t sequence_low24_[RF_SEQUENCE_MAX_FRAMES]{};
  uint16_t sequence_remote_id_[RF_SEQUENCE_MAX_FRAMES]{};
  uint16_t sequence_edges_[RF_SEQUENCE_MAX_FRAMES][RX_MAX_EDGES]{};
  uint8_t sequence_levels_[RF_SEQUENCE_MAX_FRAMES][RX_MAX_EDGES]{};
  char sequence_last_summary_[96]{"No sequence captured"};

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
  bool transmit_raw_edge_capture_(const uint16_t *edges, const uint8_t *levels, uint16_t edge_count, uint8_t initial_level, uint8_t repeats, const char *label);
  bool transmit_learned_raw_outprize_(uint8_t repeats = 1);
  bool transmit_sequence_raw_(uint8_t repeats = 1);
  bool transmit_raw_edge_capture_stx882_(const uint16_t *edges, const uint8_t *levels, uint16_t edge_count, uint8_t initial_level, uint8_t repeats, const char *label);
  bool transmit_sequence_raw_stx882_(uint8_t repeats = 1);
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

  static constexpr uint16_t SRX882_MAX_EDGES = 2048;
  bool srx882_capture_valid_{false};
  uint16_t srx882_edge_count_{0};
  uint8_t srx882_initial_level_{0};
  uint32_t srx882_capture_duration_us_{0};
  uint16_t srx882_edges_[SRX882_MAX_EDGES]{};
  uint8_t srx882_levels_[SRX882_MAX_EDGES]{};
  char srx882_summary_[128]{"No SRX882 capture"};
  char rf_recorder_status_[64]{"Ready"};
  uint32_t rf_recording_number_{0};

  bool outprize_template_valid_{true};
  uint16_t outprize_template_data_start_{0};
  TxFrameMode outprize_template_mode_{TxFrameMode::MSB_NORMAL};
  uint16_t outprize_template_pulse_us_{0};
  uint16_t outprize_template_short_gap_us_{0};
  uint16_t outprize_template_long_gap_us_{0};
  uint32_t outprize_template_source_low24_{0};
  uint16_t outprize_template_score_{0};
  char outprize_template_summary_[160]{"Built-in canonical | start=8 mode=MSB_NORMAL pulse=500 short=500 long=1500"};

  bool outprize_template_bit_(uint64_t frame, uint8_t bit_index, TxFrameMode mode) const;
  void log_rf_recording_edges_() const;

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
