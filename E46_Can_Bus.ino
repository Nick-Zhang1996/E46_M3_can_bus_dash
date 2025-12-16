#include "can_message_handler.hpp"
#include "car_status.hpp"
#include "display.hpp"
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

// On Arduino Uno R4 WiFi, Serial is USB, Serial1 is D0/D1.
// Serial is for debugging, Serial1 is for Nextion display
// However, sparkfun can bus library doesn't work with renesas CPI :()

void setup() {
  // Serial.begin(115200);
  Serial.begin(115200);
  // Serial.println("CAN Read - Testing receival of CAN Bus message");
  delay(1000);

  if (Canbus.init(CANSPEED_500)) {
    // Serial.println("CAN Init ok");
    delay(1);
  } else {
    // Serial.println("Can't init CAN");
    delay(1000);
  }
}

void dump_can_msg(tCAN &message) {
  // Serial.write(message.raw, sizeof(message.raw));
}

void print_can_msg(tCAN &message) {
  // if(message.id == 0x620 and message.data[2] == 0xFF)  //uncomment when you
  // want to filter
  //{
  // Serial.print("ID: ");
  // Serial.print(message.id, HEX);
  // Serial.print(", ");
  // Serial.print("Data: ");
  // Serial.print(message.header.length, DEC);
  // Serial.print(" ");
  for (int i = 0; i < message.header.length; i++) {
    // Serial.print(message.data[i], HEX);
    // Serial.print(" ");
  }
  // Serial.println("");
  // }
}

CarStatus status;
tCAN message;
Nextion display(Serial);

void print_car_status(CarStatus &status) {
  char buf[256];

  snprintf(buf, sizeof(buf),
           "RPM:%4d | Thr:%3d%% | Brk: %s | ST: %d | Oil:%3dF | Water:%3dF | "
           "Spd:%3dmp | "
           "Whl:%3d/%3d/%3d/%3d | Flg:%s%s",
           status.rpm.value, (int)status.throttle.value,
           status.brake_pedal.value ? "Yes" : "-",
           (int)status.steering_angle_deg.value, status.oil_temp_f.value,
           status.water_temp_f.value, (int)status.speed_mph.value,
           (int)status.wheel_speed_mph_fl.value,
           (int)status.wheel_speed_mph_fr.value,
           (int)status.wheel_speed_mph_rl.value,
           (int)status.wheel_speed_mph_rr.value,
           status.check_engine.value ? "CEL" : "-", // Simple 1-char flags
           status.overheat.value ? "Overheat" : "-");

  // Serial.println(buf);
}
float fmod(float a, float b) { return a - (int)a / (int)b * b; }

unsigned long print_update_ts = 0;
void update_test_status(CarStatus &status) {
  status.oil_temp_f.value += 10;
  status.oil_temp_f.value %= 240;
  status.water_temp_f.value += 10;
  status.water_temp_f.value %= 240;
  status.throttle.value = fmod(status.throttle.value + 10.0, 110);
  // status.brake.value++;
  // status.brake.value %= 110;
  status.brake_pedal.value = !status.brake_pedal.value;
  status.steering_angle_deg.value =
      fmod(status.steering_angle_deg.value + 10.0 + 100.0, 200) - 100;
  status.speed_mph.value = fmod(status.speed_mph.value + 10.0, 160);
  status.rpm.value = (status.rpm.value + 100) % 8500;
}
void loop() {
  if (mcp2515_check_message()) {
    if (mcp2515_get_message(&message)) {
      dispatch_can_handler(message, status);
    }
  }
  if (millis() - print_update_ts > 200) {
    print_update_ts = millis();
    print_car_status(status);
    update_test_status(status);
    display.update_display(status);
  }
}
