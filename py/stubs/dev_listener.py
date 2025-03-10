import serial
import struct

# Use the corresponding serial port for listening (e.g., /dev/pts/4)
UART_DEVICE = "/dev/pts/6"  
BAUDRATE = 921600

PACKET_SIZE = 20  # 4-byte sync + 4-byte int + 3 floats

with serial.Serial(UART_DEVICE, BAUDRATE, timeout=1) as ser:
    print(f"Listening on {UART_DEVICE}...")
    while True:
        data = ser.read(PACKET_SIZE)
        if len(data) == PACKET_SIZE:
            sync, packet_count, x, y, z = struct.unpack("!4sIfff", data)
            if sync == b"\x7F\xF0\x1C\xAF":
                print(f"Received: Count={packet_count}, X={x}, Y={y}, Z={z}")
            else:
                print("Invalid sync pattern detected!")
                exit
