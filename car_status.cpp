// Car status

template <typename T> struct Field {
  std::string_view name;
  unsigned long ts{0};
  T value{};
  void update(T new_value) {
    value = new_value;
    ts = millis();
    Serial.print(name);
    Serial.print(": ");
    Serial.println("value");
  }
}

class CarStatus {
public:
  // Temperatures
  Field<int> oil_temp_c{"Oil Temp (C)"};
  Field<int> oil_temp_f{"Oil Temp (F)"};
  Field<int> water_temp_c{"Water Temp (C)"};
  Field<int> water_temp_f{"Water Temp (F)"};

  // Engine
  Field<int> rpm{"RPM"};
  Field<float> throttle_pct{"Throttle (%)"};

  // Vehicle dynamics
  Field<float> speed_kph{"Speed (kph)"};
  Field<float> speed_mph{"Speed (mph)"};
  Field<float> steering_angle_deg{"Steering Angle (deg)"};
  Field<int8_t> lat_accel_raw{"Lateral Accel (raw)"};
  Field<float> wheel_speed_kph_fl{"Wheel Speed FL (kph)"};
  Field<float> wheel_speed_kph_fr{"Wheel Speed FR (kph)"};
  Field<float> wheel_speed_kph_rl{"Wheel Speed RL (kph)"};
  Field<float> wheel_speed_kph_rr{"Wheel Speed RR (kph)"};
  Field<float> wheel_speed_mph_fl{"Wheel Speed FL (mph)"};
  Field<float> wheel_speed_mph_fr{"Wheel Speed FR (mph)"};
  Field<float> wheel_speed_mph_rl{"Wheel Speed RL (mph)"};
  Field<float> wheel_speed_mph_rr{"Wheel Speed RR (mph)"};

  // Switches / flags
  Field<bool> brake_pedal{"Brake Pedal"};
  Field<bool> kickdown{"Kickdown"};
  Field<bool> check_engine{"Check Engine"};
  Field<bool> overheat{"Overheat"};
  Field<bool> ac_on{"AC On"};

  // Odometer / fuel
  Field<uint32_t> odometer_km{"Odometer (km)"};
  Field<uint16_t> fuel_cons_raw{"Fuel Consumption (raw)"};
};
