def parse_can_msg(frame):
    """
    Dispatcher function: checks the ID and calls the appropriate parser.
    Returns a dictionary of parsed values or None if ID is unknown.
    """
    parsers = {
        0x1F0: parse_1F0_wheel_speed, # good
        0x316: parse_316_rpm, # good
        0x329: parse_329_temp_cruise, # good
        0x545: parse_545_lights_oil, # good
        0x613: parse_613_odometer,
        0x615: parse_615_ac_temp,
        # 0x153 is documented in your text, but your data shows 0x1F3. 
        # I've implemented 0x153 based on your notes just in case.
        0x153: parse_153_speed, # good
        0x1F5: parse_1F5_steering, # good
        0x1F3: parse_1F3_lateral_g, # ???
    }
    # ID: 0x1F3, count: 8904

    if frame.id in parsers:
        try:
            return parsers[frame.id](frame.data)
        except Exception as e:
            return {"error": f"Parse error: {e}"}
    else:
        # Return None or a generic dict for unknown IDs
        return None 

# --- Individual Parsing Functions ---

def parse_1F0_wheel_speed(data):
    # Wheel speed is 12 bits per wheel. 
    # Logic: (Byte0 + (LowNibble(Byte1) * 256)) / 16
    
    # Wheel 1
    w1_raw = data[0] + ((data[1] & 0x0F) << 8)
    w1_kph = w1_raw / 16.0
    
    # Wheel 2
    w2_raw = data[2] + ((data[3] & 0x0F) << 8)
    w2_kph = w2_raw / 16.0
    
    # Wheel 3
    w3_raw = data[4] + ((data[5] & 0x0F) << 8)
    w3_kph = w3_raw / 16.0
    
    # Wheel 4
    w4_raw = data[6] + ((data[7] & 0x0F) << 8)
    w4_kph = w4_raw / 16.0

    return {
        "name": "ABS_WheelSpeed",
        "LF_Speed_kph": w1_kph,
        "RF_Speed_kph": w2_kph,
        "LR_Speed_kph": w3_kph,
        "RR_Speed_kph": w4_kph
    }

def parse_316_rpm(data):
    # RPM=(hex2dec("byte3"&"byte2"))/6.4
    # "byte3"&"byte2" implies Byte 3 is MSB, Byte 2 is LSB
    rpm_raw = (data[3] << 8) | data[2]
    rpm = rpm_raw / 6.4
    return {
        "name": "DME_RPM",
        "RPM": round(rpm, 2)
    }

def parse_329_temp_cruise(data):
    # Temp in C = .75 * hex2dec(byte01) - 48.373
    coolant_temp = (data[1] * 0.75) - 48.373
    
    # Throttle position B5
    throttle_pos = data[5] / 254.0 * 100 # Assuming 00-FE scales to %
    
    # B6 bit flags
    brake_depressed = bool(data[6] & 1)
    kickdown = bool(data[6] & 4)
    
    return {
        "name": "DME_EngineData",
        "CoolantTemp_C": round(coolant_temp, 1),
        "Throttle_Pct": round(throttle_pos, 1),
        "BrakePedal": brake_depressed,
        "Kickdown": kickdown
    }

def parse_545_lights_oil(data):
    # Oil Temp [Temp in C = hex2dec(byte04) - 48.373]
    oil_temp = data[4] - 48.373
    
    # Check Engine Light (Byte 0, binary 10 -> bit 1)
    check_engine = bool(data[0] & 0b00000010)
    overheat = bool(data[3] & 0x08)
    
    # Fuel Consumption (Byte 2 MSB, Byte 1 LSB)
    fuel_cons = (data[2] << 8) | data[1] # keeps increasing until overflow

    return {
        "name": "DME_Status",
        "OilTemp_C": round(oil_temp, 1),
        "CheckEngine": check_engine,
        "Overheat": overheat,
        "FuelCons_Raw": fuel_cons
    }

def parse_613_odometer(data):
    # Odometer MSB B1, LSB B0. * 10 = km
    odo_km = ((data[1] << 8) | data[0]) * 10
    
    # Fuel Level B2
    fuel_level_hex = data[2]
    
    # Running clock B4 (MSB), B3 (LSB) minutes
    clock_mins = (data[4] << 8) | data[3]
    
    return {
        "name": "IKE_Odometer",
        "Odometer_km": odo_km,
        "FuelLevel_Hex": hex(fuel_level_hex),
        "RunningClock_Mins": clock_mins
    }

def parse_615_ac_temp(data):
    # Outside Air Temp B3
    # x being temperature in Deg C, (x>=0 deg C,DEC2HEX(x),DEC2HEX(-x)+128)
    raw_temp = data[3]
    if raw_temp > 128:
        # Negative temp logic
        outside_temp = -(raw_temp - 128)
    else:
        outside_temp = raw_temp
        
    ac_on = (data[0] == 0x80)
    
    return {
        "name": "IKE_Climate",
        "OutsideTemp_C": outside_temp,
        "AC_On": ac_on
    }

def parse_153_speed(data):
    # B1 Speed LSB, B2 Speed MSB -> Byte 2 is MSB
    speed_raw = (data[2] << 8) | data[1]
    speed_kmh = speed_raw / 128.0
    
    return {
        "name": "ASC_Speed",
        "Speed_kmh": speed_kmh
    }
def parse_1F5_steering(data):
    # Standard E46 Steering Angle
    # Bytes 0 and 1 are the angle, Little Endian, Signed.
    raw_angle = ((data[1] & 0b01111111) << 8) | data[0]
    sign = data[1] >> 7
    if sign:
        raw_angle = -raw_angle
    
    angle_deg = raw_angle * 0.043945
    return {
        "name": "LWS_Steering",
        "Angle_Deg": round(angle_deg, 2)
    }
def parse_1F3_lateral_g(data):
    # Byte 0,1,2 is always 02, BD, 00
    # 3,4,5 changes, 
    # 4 is a bit field. 
    # 5 seems like a low resolution 8 position sensor
    # Byte 3 contains the Lateral Acceleration (G-Force)
    # It is a signed 8-bit integer.
    raw_lat = (data[3] + 128) % 256 - 128
    raw_4 = (data[4] + 128) % 256 - 128
    raw_5 = (data[5] + 128) % 256 - 128
    # Scaling: This needs verification, but typically these are 
    # scaled by roughly 0.01 to get Gs or m/s^2.
    # For now, we will plot the raw signed value to see the curve.

    return {
        "name": "DSC_LatAccel",
        "LatAccel_Raw": raw_lat,
        "byte4": raw_4,
        "byte5": raw_5
    }