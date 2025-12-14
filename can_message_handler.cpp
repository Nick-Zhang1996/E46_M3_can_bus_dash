#include "can_message_handler.hpp"
#include <Arduino.h>
#include <mcp2515.h>

constexpr int NUM_HANDLERS = 7;

CanMessageHandler *handlers[NUM_HANDLERS] = {new OilTempAndStatusHandler(),
                                             new RpmHandler(),
                                             new WaterTempAndThrottlleHandler(),
                                             new SpeedHandler(),
                                             new SteeringHandler(),
                                             new LateralGHandler(),
                                             new WheelSpeedHandler()};

void dispatch_can_handler(const tCAN &msg, CarStatus &status) {
  for (auto *h : handlers) {
    if (h->id() == msg.id) {
      h->parse(msg, status);
      return;
    }
  }
}
