#pragma once

#include <cstdint>
#include <string>
#include "codec.h"

namespace esphome {
namespace rfbridge {

class RFBridgeComponent;

class OutprizeCodec : public RFBridgeCodec {
 public:
  explicit OutprizeCodec(RFBridgeComponent *bridge) : bridge_(bridge) {}

  const char *id() const override { return "outprize"; }
  uint16_t version() const override { return 1; }
  const char *rx_backend() const override { return "cc1101"; }
  const char *tx_backend() const override { return "stx882"; }
  const char *diagnostic_rx_backend() const override { return "srx882"; }
  std::string capability_summary() const override;

  bool send_low24(uint32_t remote_id, uint32_t low24, uint8_t repeats = 1);
  bool send_complete_state(uint32_t remote_id, bool powered, uint8_t speed_percent,
                           bool direction_in, bool rain_enabled, uint8_t vent_command,
                           uint8_t repeats = 1);

 private:
  RFBridgeComponent *bridge_;
};

}  // namespace rfbridge
}  // namespace esphome
