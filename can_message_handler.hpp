#include "car_status.hpp"
#include <Arduino.h>
#include <mcp2515.h>

class CanMessageHandler {
public:
  // CAN BUS ID of the message i.e. 0x153
  virtual uint16_t id() const = 0;
  // Parse [msg], update [status], Serial print debug info
  virtual void parse(const tCAN &msg, CarStatus &status) = 0;
  virtual ~CanMessageHandler() = default;
};

class RpmHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x316;
  uint16_t id() const override { return ARBID; }

  void parse(const tCAN &msg, CarStatus &status) override {
    int rpm = (msg.data[3] << 8) | msg.data[2];
    status.rpm.update(rpm);
  }
};

class OilTempAndStatusHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x545;
  uint16_t id() const override { return ARBID; }

  void parse(const tCAN &msg, CarStatus &status) override {
    float oil_temp_c = msg.data[4] - 48;
    status.oil_temp_c.update(oil_temp_c);
    status.oil_temp_f.update(oil_temp_c * 1.8 + 32.0);

    // Check Engine: bit 1 of byte 0
    status.check_engine.update(msg.data[0] & 0x02);

    // Overheat: bit 3 of byte 3
    status.overheat.update(msg.data[3] & 0x08);

    // Fuel consumption raw (byte2 MSB, byte1 LSB)
    uint16_t fuel = (msg.data[2] << 8) | msg.data[1];
    status.fuel_cons_raw.update(fuel);
  }
};

class WaterTempAndThrottlleHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x329;
  uint16_t id() const override { return ARBID; }

  void parse(const tCAN &msg, CarStatus &status) override {
    int water_temp_c = (msg.data[1] * 0.75f) - 48;
    status.water_temp_c.update(water_temp_c);
    status.water_temp_f.update(water_temp_c * 1.8 + 32.0);

    int throttle = msg.data[5] / 254.0f * 100.0f;
    status.throttle_pct.update(throttle);

    status.brake_pedal.update(msg.data[6] & 0x01);
    status.kickdown.update(msg.data[6] & 0x04);
  }
};

class SpeedHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x153;
  uint16_t id() const override { return ARBID; }

  void parse(const tCAN &msg, CarStatus &status) override {
    uint16_t raw = (msg.data[2] << 8) | msg.data[1];
    float speed_kph = raw / 128.0f;
    status.speed_kph.update(speed_kph);
    status.speed_mph.update(speed_kph * 0.621371f);
  }
};

class SteeringHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x1F5;
  uint16_t id() const override { return ARBID; }

  void parse(const tCAN &msg, CarStatus &status) override {
    int16_t raw = ((msg.data[1] & 0x7F) << 8) | msg.data[0];
    if (msg.data[1] & 0x80)
      raw = -raw;

    float angle = raw * 0.043945f;
    status.steering_angle_deg.update(angle);
  }
};

class LateralGHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x1F3;
  uint16_t id() const override { return ARBID; }

  static int8_t signed_byte(uint8_t b) { return static_cast<int8_t>(b); }

  void parse(const tCAN &msg, CarStatus &status) override {
    int8_t lat = signed_byte(msg.data[3]);
    status.lat_accel_raw.update(lat);
  }
};

class WheelSpeedHandler : public CanMessageHandler {
public:
  static constexpr uint16_t ARBID = 0x1F0;
  uint16_t id() const override { return ARBID; }
  void parse(const tCAN &msg, CarStatus &status) override {
    int w1_raw = msg.data[0] + ((msg.data[1] & 0x0F) << 8);
    int w1_kph = w1_raw / 16.0;
    int w1_mph = w1_raw / 10.0;

    int w2_raw = msg.data[2] + ((msg.data[3] & 0x0F) << 8);
    int w2_kph = w2_raw / 16.0;
    int w2_mph = w2_raw / 10.0;

    int w3_raw = msg.data[4] + ((msg.data[5] & 0x0F) << 8);
    int w3_kph = w3_raw / 16.0;
    int w3_mph = w3_raw / 10.0;

    int w4_raw = msg.data[6] + ((msg.data[7] & 0x0F) << 8);
    int w4_kph = w4_raw / 16.0;
    int w4_mph = w4_raw / 10.0;

    status.wheel_speed_kph_fl.update(w1_kph);
    status.wheel_speed_kph_fr.update(w2_kph);
    status.wheel_speed_kph_rl.update(w3_kph);
    status.wheel_speed_kph_rr.update(w4_kph);

    status.wheel_speed_mph_fl.update(w1_mph);
    status.wheel_speed_mph_fr.update(w2_mph);
    status.wheel_speed_mph_rl.update(w3_mph);
    status.wheel_speed_mph_rr.update(w4_mph);
  }
};

void dispatch_can_handler(const tCAN &msg, CarStatus &status);