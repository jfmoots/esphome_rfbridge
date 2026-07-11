#pragma once

// Header-included implementation for ESPHome external-component builds.
// ESPHome 2026.6.x only compiles the component's primary rfbridge.cpp translation
// unit, so codec definitions are included there explicitly.

namespace esphome {
namespace rfbridge {

inline std::string OutprizeCodec::capability_summary() const {
  return "outprize:v1 rx=cc1101 tx=stx882 diagnostic_rx=srx882";
}

inline bool OutprizeCodec::send_low24(uint32_t remote_id, uint32_t low24, uint8_t repeats) {
  return this->bridge_ != nullptr && this->bridge_->send_outprize_low24(remote_id, low24, repeats);
}

inline bool OutprizeCodec::send_complete_state(uint32_t remote_id, bool powered, uint8_t speed_percent,
                                                bool direction_in, bool rain_enabled, uint8_t vent_command,
                                                uint8_t repeats) {
  if (this->bridge_ == nullptr) return false;
  if (!powered) return this->bridge_->send_outprize_low24(remote_id, 0x600000UL, repeats);
  const auto direction = direction_in ? OutprizeDirection::IN : OutprizeDirection::OUT;
  const auto vent = static_cast<OutprizeVentCommand>(vent_command & 0x0C);
  const uint32_t low24 = this->bridge_->encode_outprize_low24(speed_percent, direction, rain_enabled, vent);
  return this->bridge_->send_outprize_low24(remote_id, low24, repeats);
}

}  // namespace rfbridge
}  // namespace esphome
