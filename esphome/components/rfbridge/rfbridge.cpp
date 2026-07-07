#include "rfbridge.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rfbridge {

static const char *const TAG = "rfbridge";

void RFBridgeComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RF Bridge...");
  // TODO: initialize CC1101 registers.
}

void RFBridgeComponent::loop() {
  // TODO: receive/decode path.
}

void RFBridgeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RF Bridge");
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
      // Round to nearest supported 10% step.
      return this->outprize_speed_code_((speed_percent + 5) / 10 * 10);
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
  // TODO: implement actual CC1101 OOK transmit.
  // Packet to transmit is conceptually: remote_id (24 bits) + low24/state bits.
  ESP_LOGW(TAG, "TX placeholder only; CC1101 transmit not implemented yet.");
  return false;
}

}  // namespace rfbridge
}  // namespace esphome
