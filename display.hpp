#ifndef __display_hpp__
#define __display_hpp__
#include "car_status.hpp"
#include <Arduino.h>

#define BLACK 32
#define GREEN 2016
#define RED 63488
#define BLUE 1023

class Nextion {
public:
  Nextion(HardwareSerial &serial) : serial_(serial), buffer_{} {}

  void send_command(char *b) {
    serial_.print(b);
    serial_.write(0xFF);
    serial_.write(0xFF);
    serial_.write(0xFF);
  }

  void set_water_temp_f(int val) {
    sprintf(buffer_, "water.txt=\"%d F\"", val);
    send_command(buffer_);
    // unit:F min: 32, low: 158, high:221, max:248
    // unit:C min: 0 , low: 70 , high:105, max:120
    // 32F = 0C, 212F = 100C
    // Nextion colors: black 32, green 2016, red 63488, blue 1023

    int bar_val = map(val, 32, 248, 1, 100);
    bar_val = constrain(bar_val, 1, 100);
    sprintf(buffer_, "water_bar.val=%d", bar_val);
    send_command(buffer_);
    if (val < 158) {
      send_command("water_bar.pco=1023");
    } else if (val > 221) {
      send_command("water_bar.pco=63488");
    } else {
      send_command("water_bar.pco=2016");
    }
  }

  void set_oil_temp_f(int val) {
    sprintf(buffer_, "oil_temp.txt=\"%d F\"", val);
    send_command(buffer_);
    // M3 gauge: 120, 210, 300 F, mid dots: 165, 255
    // 50, 100, 150 C
    // unit:F min: 120, low: 165, high:255, max:300
    // unit:C min: 50 , low: 75 , high:125, max:150
    // 32F = 0C, 212F = 100C

    int bar_val = map(val, 120, 300, 1, 100);
    bar_val = constrain(bar_val, 1, 100);
    sprintf(buffer_, "oil_temp_bar.val=%d", bar_val);
    send_command(buffer_);
    if (val < 165) {
      send_command("oil_temp_bar.pco=1023");
    } else if (val > 255) {
      send_command("oil_temp_bar.pco=63488");
    } else {
      send_command("oil_temp_bar.pco=2016");
    }
  }

  void set_rpm(int val) {
    sprintf(buffer_, "rpm.txt=\"%d\"", val);
    send_command(buffer_);
    int bar_val = map(val, 1, 8000, 1, 100);
    bar_val = constrain(bar_val, 1, 100);
    sprintf(buffer_, "rpm_bar.val=%d", bar_val);
    send_command(buffer_);
    if (val > 7000) {
      send_command("rpm_bar.pco=63488");
    } else {
      send_command("rpm_bar.pco=2016");
    }
  }

  // val: 1-100
  void set_throttle(int val) {
    sprintf(buffer_, "throttle.txt=\"%d\"", val);
    send_command(buffer_);
    int bar_val = constrain(val, 1, 100);
    sprintf(buffer_, "throttle_bar.val=%d", bar_val);
    send_command(buffer_);
  }

  // val: 1-100
  void set_brake(int val) {
    sprintf(buffer_, "brake.txt=\"%d\"", val);
    send_command(buffer_);
    int bar_val = constrain(val, 1, 100);
    sprintf(buffer_, "brake_bar.val=%d", bar_val);
    send_command(buffer_);
  }

  // val: -100-100
  void set_steering(int val) {
    sprintf(buffer_, "steering.txt=\"%d\"", val);
    send_command(buffer_);
    int bar_val = map(val, -100, 100, 1, 100);
    bar_val = constrain(bar_val, 1, 100);
    sprintf(buffer_, "steering_bar.val=%d", bar_val);
    send_command(buffer_);
  }

  // unit: mph
  void set_speed(int val) {
    sprintf(buffer_, "mph.val=%d", val);
    send_command(buffer_);
  }

  void update_display(CarStatus &status) {
    set_water_temp_f(status.water_temp_f.value);
    set_oil_temp_f(status.oil_temp_f.value);
    set_rpm(status.rpm.value);
    set_throttle(status.throttle.value);
    set_steering(status.steering_angle_deg.value);
    set_brake(status.brake_pedal.value ? 100 : 0);
    set_speed(status.speed_mph.value);
  }

private:
  HardwareSerial &serial_;
  char buffer_[128];
  char buffer_f_[32]; // buffer for float number
};

#endif