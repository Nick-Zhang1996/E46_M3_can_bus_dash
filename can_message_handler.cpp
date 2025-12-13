#include <mcp2515.h>
#include <Arduino.h>

class CanMessageHandler {
    public:
    virtual uint16_t id() const = 0;
    virtual void parse(const tCAN& msg) = 0;
};

class OilTempHandler : public CanMessageHandler{
    static constexpr uint16_t ARBID = 0x545;
    uint16_t id() const override { return ARBID;}
    void parse(const tCAN& msg){
        int oil_temp_in_c = msg.data[4] - 48;
        Serial.print("Oil Temp (C): ");
        Serial.println(oil_temp_in_c);
    }
};

class RpmHandler : public CanMessageHandler {
    static constexpr uint16_t ARBID = 0x316;
    uint16_t id() const override { return ARBID;}
    void parse(const tCAN& msg){
        int rpm = msg.data[3] << 8 + msg.data[2];
        Serial.print("RPM: ");
        Serial.println(rpm);
    }
};