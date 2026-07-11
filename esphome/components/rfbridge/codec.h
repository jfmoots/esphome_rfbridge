#pragma once

#include <cstdint>
#include <string>

namespace esphome {
namespace rfbridge {

class RFBridgeCodec {
 public:
  virtual ~RFBridgeCodec() = default;
  virtual const char *id() const = 0;
  virtual uint16_t version() const = 0;
  virtual const char *rx_backend() const = 0;
  virtual const char *tx_backend() const = 0;
  virtual const char *diagnostic_rx_backend() const { return "none"; }
  virtual std::string capability_summary() const = 0;
};

}  // namespace rfbridge
}  // namespace esphome
