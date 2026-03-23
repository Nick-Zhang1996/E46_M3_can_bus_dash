
#include <Arduino.h>

class Hella {
private:
  // Must be an interrupt-capable pin (e.g., Pin 2 on Arduino Uno)
  const uint8_t SENSOR_PIN;
  // Volatile variables for ISR timing
  volatile unsigned long last_rising_time = 0;
  volatile unsigned long width_S1 = 0, width_T1 = 0, width_T2 = 0;
  volatile unsigned long period_S1 = 0, period_T1 = 0, period_T2 = 0;
  volatile int pulse_index = -1; // 0 = S1, 1 = T1, 2 = T2
  volatile bool new_data_ready = false;

  float temp_c = 0;
  float pressure_bar = 0;
  char diag_status[32];
  bool is_working_ = true;

public:
  Hella(uint8_t sensor_pin) : SENSOR_PIN(sensor_pin) {
    pinMode(SENSOR_PIN, INPUT); // External 10k pull-up required per datasheet
    // Attach interrupt on CHANGE to capture both rising and falling edges
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensorISR, CHANGE);
  }

  void isr() {
    unsigned long current_time = micros();
    bool state = digitalRead(SENSOR_PIN);

    if (state == HIGH) { // --- RISING EDGE ---
      unsigned long delta_rising = current_time - last_rising_time;
      last_rising_time = current_time;

      // Determine position in the PWM cycle based on the time since the last
      // rising edge S1 window is ~1024us, T1 window is ~4096us, T2 window is
      // ~4096us
      if (delta_rising > 500 && delta_rising < 1500) {
        // Just finished the 1024us window. The last pulse was S1, current is
        // T1.
        period_S1 = delta_rising;
        pulse_index = 1;
      } else if (delta_rising > 3000 && delta_rising < 5000) {
        if (pulse_index == 1) {
          // Just finished the first 4096us window. Last was T1, current is T2.
          period_T1 = delta_rising;
          pulse_index = 2;
        } else if (pulse_index == 2) {
          // Just finished the second 4096us window. Last was T2, current is
          // next S1.
          period_T2 = delta_rising;
          pulse_index = 0;
          new_data_ready = true; // A full cycle (S1, T1, T2) is complete
        } else {
          pulse_index = -1; // Out of sync
        }
      } else {
        pulse_index = -1; // Invalid timing, reset sync
      }

    } else { // --- FALLING EDGE ---
      unsigned long pulse_width = current_time - last_rising_time;

      if (pulse_index == 0) {
        width_S1 = pulse_width;
      } else if (pulse_index == 1) {
        width_T1 = pulse_width;
      } else if (pulse_index == 2) {
        width_T2 = pulse_width;
      }
    }
  }

  void update() {
    if (!new_data_ready) {
      return;
    }
    // 1. Safely grab the volatile variables
    cli();
    uint16_t t1_width = width_T1;
    uint16_t t1_period = period_T1;
    uint16_t t2_width = width_T2;
    uint16_t t2_period = period_T2;
    uint16_t s1_width = width_S1;
    uint16_t s1_period = period_S1;
    new_data_ready = false;
    sei();

    // Prevent division by zero if glitch occurs
    if (t1_period > 0 && t2_period > 0 && s1_period > 0) {

      // 2. Temperature Calculation (Celsius)
      // Formula: Temp = ((4096 / period_T1) * width_T1 - 128) * (1 / 19.2) -
      // 40
      temp_c = ((4096.0 / t1_period) * t1_width - 128.0) * (1.0 / 19.2) - 40.0;

      // 3. Pressure Calculation (Bar -> PSI)
      // Pressure(bar) = ((4096 / period_T2) * width_T2 - 128) * (1 / 384) +
      // 0.5
      pressure_bar =
          ((4096.0 / t2_period) * t2_width - 128.0) * (1.0 / 384.0) + 0.5;

      // 4. Diagnostic State Calculation
      float dc_s1 = (1024.0 / s1_period) * s1_width;
      if (dc_s1 >= 230 && dc_s1 <= 280) {
        if (!is_working_) {
          sprintf(diag_status, "OK (Functional)");
          is_working_ = true;
        }
      } else if (dc_s1 >= 360 && dc_s1 <= 410) {
        sprintf(diag_status, "Pressure Failure");
        is_working_ = false;
      } else if (dc_s1 >= 485 && dc_s1 <= 535) {
        sprintf(diag_status, "Temperature Failure");
        is_working_ = false;
      } else if (dc_s1 >= 615 && dc_s1 <= 665) {
        sprintf(diag_status, "Hardware Failure");
        is_working_ = false;
      } else {
        sprintf(diag_status, "Unknown");
        is_working_ = false;
      }
    }
  }
  float get_pressure_bar() { return pressure_bar; }
  float get_pressure_psi() { return pressure_bar * 14.5037738; }
  float get_temp_c() { return temp_c; }
  float get_temp_f() { return temp_c * 1.4 + 32.0; }
  bool is_working() { return is_working_; }
};

extern Hella g_hella;

// Interrupt Service Routine
void sensorISR() { g_hella.isr(); }