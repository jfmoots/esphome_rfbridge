#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

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

class RFBridgeComponent : public Component,
                          public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  uint32_t encode_outprize_low24(uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                                 OutprizeVentCommand vent_command) const;

  bool send_outprize(uint32_t remote_id, uint8_t speed_percent, OutprizeDirection direction, bool rain_enabled,
                     OutprizeVentCommand vent_command);

 protected:
  uint8_t outprize_speed_code_(uint8_t speed_percent) const;
  bool transmit_low24_(uint32_t remote_id, uint32_t low24);
};

}  // namespace rfbridge
}  // namespace esphome
