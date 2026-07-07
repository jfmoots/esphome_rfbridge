#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#define PIN_CC1101_CS    5
#define PIN_CC1101_GDO0  4
#define PIN_CC1101_RST   RADIOLIB_NC
#define PIN_CC1101_GDO2  RADIOLIB_NC

#define PIN_SPI_SCK      18
#define PIN_SPI_MISO     19
#define PIN_SPI_MOSI     23

// CC1101 register addresses
#define CC1101_IOCFG2    0x00
#define CC1101_IOCFG0    0x02
#define CC1101_PKTCTRL0  0x08

// CC1101 values
#define GDO_SERIAL_DATA  0x0D   // async serial data output
#define GDO_HIGH_Z       0x2E
#define PKT_ASYNC_SERIAL 0x32   // async serial mode

CC1101 radio = new Module(PIN_CC1101_CS, PIN_CC1101_GDO0, PIN_CC1101_RST, PIN_CC1101_GDO2);

constexpr uint16_t MAX_EDGES = 512;

constexpr float RSSI_ARM_DBM = -80.0;
constexpr uint32_t CAPTURE_WINDOW_US = 140000;
constexpr uint32_t CAPTURE_COOLDOWN_MS = 500;

constexpr uint16_t OUTPRIZE_MIN_EDGES = 30;
constexpr uint16_t OUTPRIZE_MAX_EDGES = 95;

// Do NOT name these LONG_MIN/LONG_MAX.
// Arduino/ESP32 headers already define LONG_MIN and LONG_MAX.
constexpr uint16_t SHORT_US_MIN = 350;
constexpr uint16_t SHORT_US_MAX = 750;
constexpr uint16_t LONG_US_MIN  = 1150;
constexpr uint16_t LONG_US_MAX  = 1800;
constexpr uint16_t SYNC_US_MIN  = 3800;
constexpr uint16_t SYNC_US_MAX  = 5200;
constexpr uint16_t RESET_US_MIN = 7500;
constexpr uint16_t RESET_US_MAX = 10000;

void cc1101_write_reg(uint8_t addr, uint8_t value) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CC1101_CS, LOW);
  delayMicroseconds(5);
  SPI.transfer(addr);
  SPI.transfer(value);
  delayMicroseconds(5);
  digitalWrite(PIN_CC1101_CS, HIGH);
  SPI.endTransaction();
}

uint8_t cc1101_read_reg(uint8_t addr) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CC1101_CS, LOW);
  delayMicroseconds(5);
  SPI.transfer(addr | 0x80);
  uint8_t value = SPI.transfer(0x00);
  delayMicroseconds(5);
  digitalWrite(PIN_CC1101_CS, HIGH);
  SPI.endTransaction();
  return value;
}

void configure_gdo0_for_async_data() {
  cc1101_write_reg(CC1101_IOCFG0, GDO_SERIAL_DATA);
  cc1101_write_reg(CC1101_IOCFG2, GDO_HIGH_Z);
  cc1101_write_reg(CC1101_PKTCTRL0, PKT_ASYNC_SERIAL);

  Serial.println("Forced CC1101 GDO0 async data mode:");
  Serial.print("  IOCFG0   = 0x"); Serial.println(cc1101_read_reg(CC1101_IOCFG0), HEX);
  Serial.print("  IOCFG2   = 0x"); Serial.println(cc1101_read_reg(CC1101_IOCFG2), HEX);
  Serial.print("  PKTCTRL0 = 0x"); Serial.println(cc1101_read_reg(CC1101_PKTCTRL0), HEX);
}

bool in_range(uint16_t value, uint16_t low, uint16_t high) {
  return value >= low && value <= high;
}

char bucket(uint16_t t) {
  if (in_range(t, SHORT_US_MIN, SHORT_US_MAX)) return 'S';
  if (in_range(t, LONG_US_MIN, LONG_US_MAX)) return 'L';
  if (in_range(t, SYNC_US_MIN, SYNC_US_MAX)) return 'Y';
  if (in_range(t, RESET_US_MIN, RESET_US_MAX) || t == 65535) return 'R';
  return '?';
}

void print_raw(uint16_t count, const uint16_t *timings, const uint8_t *levels) {
  for (uint16_t i = 0; i < count; i++) {
    Serial.print(levels[i] ? "+" : "-");
    Serial.print(timings[i]);
  }
  Serial.println();
}

void print_bits(const bool *bits, uint16_t bit_count) {
  for (uint16_t i = 0; i < bit_count; i++) {
    Serial.print(bits[i] ? '1' : '0');
  }
}

void print_hex_from_bits(const bool *bits, uint16_t bit_count) {
  uint8_t nibble = 0;
  uint8_t used = 0;

  for (uint16_t i = 0; i < bit_count; i++) {
    nibble = (nibble << 1) | (bits[i] ? 1 : 0);
    used++;

    if (used == 4) {
      Serial.print(nibble, HEX);
      Serial.print(' ');
      nibble = 0;
      used = 0;
    }
  }

  if (used > 0) {
    nibble <<= (4 - used);
    Serial.print(nibble, HEX);
  }
}

uint32_t low24_from_bits(const bool *bits, uint16_t bit_count) {
  uint32_t value = 0;
  uint16_t start = bit_count > 24 ? bit_count - 24 : 0;

  for (uint16_t i = start; i < bit_count; i++) {
    value = (value << 1) | (bits[i] ? 1UL : 0UL);
  }

  return value & 0xFFFFFF;
}

bool decode_gap_width_from_index(
  uint16_t start_index,
  uint16_t count,
  const uint16_t *timings,
  const uint8_t *levels,
  float rssi
) {
  bool bits[64];
  uint16_t bit_count = 0;

  // Outprize captures are gap-width PWM:
  //   short low pulse around 500us
  //   data is in the following high gap:
  //     short gap around 500us  = 0
  //     long gap around 1500us  = 1
  //
  // Raw example:
  //   +4543 sync, then -526 +1468, -536 +1449, -532 +542...
  // The +1468/+1449/+542 gap timings are the actual bits.
  for (uint16_t i = start_index; i + 1 < count && bit_count < 64; i += 2) {
    uint16_t pulse = timings[i];
    uint16_t gap = timings[i + 1];

    // Pulse should normally be short. Be tolerant, but don't decode nonsense.
    if (!in_range(pulse, SHORT_US_MIN, SHORT_US_MAX)) {
      return false;
    }

    if (in_range(gap, SHORT_US_MIN, SHORT_US_MAX)) {
      bits[bit_count++] = false;
    } else if (in_range(gap, LONG_US_MIN, LONG_US_MAX)) {
      bits[bit_count++] = true;
    } else {
      return false;
    }
  }

  if (bit_count < 24 || bit_count > 40) {
    return false;
  }

  uint32_t low24 = low24_from_bits(bits, bit_count);

  Serial.println();
  Serial.println("===== OUTPRIZE_PACKET =====");
  Serial.println("Decoder: PWM gap short=0 long=1");
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("Edges: ");
  Serial.println(count);
  Serial.print("Bits: ");
  Serial.println(bit_count);
  Serial.print("Binary: ");
  print_bits(bits, bit_count);
  Serial.println();
  Serial.print("Hex: ");
  print_hex_from_bits(bits, bit_count);
  Serial.println();
  Serial.print("Low24: 0x");
  if (low24 < 0x100000) Serial.print('0');
  if (low24 < 0x010000) Serial.print('0');
  if (low24 < 0x001000) Serial.print('0');
  if (low24 < 0x000100) Serial.print('0');
  if (low24 < 0x000010) Serial.print('0');
  Serial.println(low24, HEX);
  Serial.print("Raw: ");
  print_raw(count, timings, levels);
  Serial.println("===========================");

  return true;
}

bool looks_like_outprize(uint16_t count, const uint16_t *timings) {
  if (count < OUTPRIZE_MIN_EDGES || count > OUTPRIZE_MAX_EDGES) {
    return false;
  }

  uint16_t shortish = 0;
  uint16_t longish = 0;
  uint16_t syncish = 0;
  uint16_t resetish = 0;

  for (uint16_t i = 0; i < count; i++) {
    char b = bucket(timings[i]);
    if (b == 'S') shortish++;
    else if (b == 'L') longish++;
    else if (b == 'Y') syncish++;
    else if (b == 'R') resetish++;
  }

  return shortish >= 18 && longish >= 6 && (syncish >= 1 || resetish >= 1);
}

void analyze_packet(uint16_t count, const uint16_t *timings, const uint8_t *levels, float rssi) {
  if (!looks_like_outprize(count, timings)) {
    return;
  }

  // Find the sync gap/pulse around 4.5ms. Data starts immediately after it.
  for (uint16_t i = 0; i < count; i++) {
    if (in_range(timings[i], SYNC_US_MIN, SYNC_US_MAX)) {
      uint16_t start = i + 1;

      // Move to the first short pulse after sync.
      while (start < count && !in_range(timings[start], SHORT_US_MIN, SHORT_US_MAX)) {
        start++;
      }

      decode_gap_width_from_index(start, count, timings, levels, rssi);
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== MooterHome RF Sniffer - Outprize Gap Decoder v3 ===");

  pinMode(PIN_CC1101_CS, OUTPUT);
  digitalWrite(PIN_CC1101_CS, HIGH);

  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_CC1101_CS);

  int state = radio.begin();
  Serial.print("radio.begin() = ");
  Serial.println(state);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("CC1101 init failed.");
    while (true) delay(1000);
  }

  Serial.print("setFrequency(433.92) = ");
  Serial.println(radio.setFrequency(433.92));

  Serial.print("setOOK(true) = ");
  Serial.println(radio.setOOK(true));

  Serial.print("setBitRate(2.0) = ");
  Serial.println(radio.setBitRate(2.0));

  Serial.print("setRxBandwidth(58.0) = ");
  Serial.println(radio.setRxBandwidth(58.0));

  Serial.print("receiveDirect() = ");
  Serial.println(radio.receiveDirect());

  configure_gdo0_for_async_data();

  pinMode(PIN_CC1101_GDO0, INPUT);

  Serial.println("Listening. Press one Outprize remote button.");
  Serial.println("Valid packets will print once per press.");
}

void loop() {
  static uint32_t last_idle = 0;
  static uint32_t last_capture_ms = 0;

  float rssi = radio.getRSSI();

  if (rssi >= RSSI_ARM_DBM && millis() - last_capture_ms > CAPTURE_COOLDOWN_MS) {
    last_capture_ms = millis();

    uint16_t local_timings[MAX_EDGES];
    uint8_t local_levels[MAX_EDGES];
    uint16_t count = 0;

    bool last_level = digitalRead(PIN_CC1101_GDO0);
    uint32_t last_edge_us = micros();
    uint32_t start_us = last_edge_us;

    while ((micros() - start_us) < CAPTURE_WINDOW_US && count < MAX_EDGES) {
      bool now_level = digitalRead(PIN_CC1101_GDO0);

      if (now_level != last_level) {
        uint32_t now_us = micros();
        uint32_t delta = now_us - last_edge_us;

        local_timings[count] = delta > 65535 ? 65535 : delta;
        local_levels[count] = last_level ? 1 : 0;
        count++;

        last_level = now_level;
        last_edge_us = now_us;
      }
    }

    analyze_packet(count, local_timings, local_levels, rssi);
  }

  if (millis() - last_idle > 5000) {
    last_idle = millis();
    Serial.print("idle rssi=");
    Serial.println(rssi);
  }

  delay(5);
}
