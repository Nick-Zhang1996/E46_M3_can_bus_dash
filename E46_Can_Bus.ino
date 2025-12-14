#include "can_message_handler.hpp"
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

void setup() {
  Serial.begin(115200); // For debug use
  // Serial.println("CAN Read - Testing receival of CAN Bus message");
  delay(1000);

  if (Canbus.init(CANSPEED_500)) // Initialise MCP2515 CAN controller at the
                                 // specified speed
    // Serial.println("CAN Init ok");
    delay(1);
  else
    // Serial.println("Can't init CAN");
    for (;;) {
      delay(100);
    }

  delay(1000);
}

void dump_can_msg(tCAN &message) {
  Serial.write(message.raw, sizeof(message.raw));
}

void print_can_msg(tCAN &message) {
  // if(message.id == 0x620 and message.data[2] == 0xFF)  //uncomment when you
  // want to filter
  //{
  Serial.print("ID: ");
  Serial.print(message.id, HEX);
  Serial.print(", ");
  Serial.print("Data: ");
  Serial.print(message.header.length, DEC);
  Serial.print(" ");
  for (int i = 0; i < message.header.length; i++) {
    Serial.print(message.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  //}
}

CarStatus status;
tCAN message;
void loop() {
  if (mcp2515_check_message()) {
    if (mcp2515_get_message(&message)) {
      dispatch_can_handler(message, status);
    }
  }
}
