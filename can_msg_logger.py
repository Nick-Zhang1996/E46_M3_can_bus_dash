import struct
import pickle
from time import sleep

import serial

SERIAL_PORT = '/dev/tty.usbmodem1201'
BAUD_RATE = 115200

class CANFrame:
    __slots__ = ('id', 'rtr', 'length', 'data')

    def __init__(self, id, rtr, length, data):
        self.id = id
        self.rtr = rtr
        self.length = length
        self.data = data  # a bytes object of length 8

    def __repr__(self):
        return f"CANFrame(id=0x{self.id:03X}, rtr={self.rtr}, length={self.length}, data={self.data.hex(' ')})"


def main():
    # Open serial port
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2e-1) as ser:
        print(f"Listening on {SERIAL_PORT} at {BAUD_RATE} baud...")
        frames = []
        try:
            while True:
                raw = ser.read(11)
                if not raw:
                    sleep(1e-2)
                    continue
                if not len(raw)==11:
                    print(f"Expected 11 bytes, got {len(raw)}")
                    continue

                # Print raw byte as hex
                print(f"{raw.hex()}", end=' ')

                # < = little endian
                # H = uint16
                # B = uint8 (header byte)
                # 8s = 8-byte string
                id_val, header_byte, data = struct.unpack("<HB8s", raw)

                # Extract bitfields
                # rtr = bit 0
                rtr = header_byte & 0b00000001

                # length = bits 1–4
                length = (header_byte >> 1) & 0b1111

                frame = CANFrame(id_val, rtr, length, data)
                print(frame)
                frames.append(frame)
        except KeyboardInterrupt:
            print(f'saving {len(frames)} frames')
            with open("can_frames.pkl", "wb") as f:
                pickle.dump(frames,f)


if __name__ == "__main__":
    main()
