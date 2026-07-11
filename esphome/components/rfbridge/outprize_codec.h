#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "codec.h"

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

// Pure protocol codec. It has no RFBridgeComponent pointer, performs no I/O,
// and knows nothing about ESPHome entities or Home Assistant. The bridge owns
// capture, routing, and radio backends; this codec owns Outprize semantics.
class OutprizeCodec : public RFBridgeCodec {
 public:
  const char *id() const override { return "outprize"; }
  uint16_t version() const override { return 1; }
  const char *rx_backend() const override { return "cc1101"; }
  const char *tx_backend() const override { return "stx882"; }
  const char *diagnostic_rx_backend() const override { return "srx882"; }

  std::string capability_summary() const override {
    return "outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882";
  }

  uint8_t normalize_speed(uint8_t speed_percent) const {
    return static_cast<uint8_t>(std::min<uint16_t>(100, ((speed_percent + 5) / 10) * 10));
  }

  uint32_t encode_low24(uint8_t speed_percent, OutprizeDirection direction,
                        bool rain_enabled, OutprizeVentCommand vent_command) const {
    uint32_t low24 = 0x600000UL | this->speed_base_(this->normalize_speed(speed_percent));
    if (direction == OutprizeDirection::IN) low24 |= 0x20;
    if (rain_enabled) low24 |= 0x10;
    low24 |= static_cast<uint8_t>(vent_command) & 0x0C;
    return low24 & 0xFFFFFFUL;
  }

  uint8_t decode_speed(uint32_t low24) const {
    switch (low24 & 0x7C0UL) {
      case 0x040: return 0;
      case 0x440: return 10;
      case 0x240: return 20;
      case 0x640: return 30;
      case 0x140: return 40;
      case 0x540: return 50;
      case 0x340: return 60;
      case 0x740: return 70;
      case 0x0C0: return 80;
      case 0x4C0: return 90;
      case 0x2C0: return 100;
      default: return 0;
    }
  }

 private:
  uint32_t speed_base_(uint8_t speed_percent) const {
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
      default: return 0x040;
    }
  }
};

}  // namespace rfbridge
}  // namespace esphome
