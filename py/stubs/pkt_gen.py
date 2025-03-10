import serial
import socket
import struct
import time

# Constants
UART_DEVICE = "/dev/pts/9"
BAUDRATE = 921600
PACKET_SIZE = 20
SYNC_PATTERN = b"\x7F\xF0\x1C\xAF"
BROADCAST_PORT = 5000
INTERVAL_MS = 80

# Function to generate a test packet
def generate_test_packet(packet_count, x, y, z):
    packet = struct.pack(
        "!4sIfff", SYNC_PATTERN, packet_count, x, y, z
    )
    return packet

# Function to simulate UART IMU output
def imu_simulator():
    with serial.Serial(UART_DEVICE, BAUDRATE, timeout=1) as ser:
        packet_count = 0
        x, y, z = 1.23, -4.56, 7.89  # Example test values
        test_packet = generate_test_packet(packet_count, x, y, z)
        ser.write(test_packet)
        packet_count += 1



if __name__ == "__main__":
    # import threading
    # simulator_thread = threading.Thread(target=imu_simulator, daemon=True)
    # simulator_thread.start()
    imu_simulator()

  