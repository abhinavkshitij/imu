"""
This module provides utility function as a runner and tester for an IMU packet processor.
"""
import struct
import socket
import json
import time
import threading
import serial # pylint: disable=import-error

# Constants
UART_DEVICE = "/dev/pts/9"  # Change this based on the actual serial port
BAUDRATE = 921600
PACKET_SIZE = 20
SYNC_PATTERN = b"\x7F\xF0\x1C\xAF"
BROADCAST_PORT = 5000
INTERVAL_MS = 80

# Function to generate a test packet (in IEEE-754 format, network byte order)
def generate_test_packet(packet_count: int, x_rate: float, y_rate: float, z_rate: float) -> None:
    """
    Packet generator for IMU sim

    @param packet_count: Packet count from the start of the simulation
    @param x_rate: x gyro rate (rad/s)
    @param y_rate: y gyro rate (rad/s)
    @param z_rate: z gyro rate (rad/s)
    @return: None
    """
    return struct.pack("!4sIfff", SYNC_PATTERN, packet_count, x_rate, y_rate, z_rate)

# Function to simulate IMU output (sends packets over UART)
def imu_simulator() -> None:
    """
    IMU Mockup as a Serial UART device
    """
    with serial.Serial(UART_DEVICE, BAUDRATE, timeout=1) as ser:
        packet_count = 0
        while True:
            x_rate, y_rate, z_rate = 1.23, -4.56, 7.89  # Example test values
            test_packet = generate_test_packet(packet_count, x_rate, y_rate, z_rate)
            ser.write(test_packet)
            print(f"Sent Packet: Count={packet_count}, X={x_rate}, Y={y_rate}, Z={z_rate}")
            packet_count += 1
            time.sleep(INTERVAL_MS / 1000.0)

# Function to listen for JSON broadcast over UDP
def listen_for_broadcast() -> None:
    """
    Python listener for JSON object broadcasted on localhost
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("127.0.0.1", BROADCAST_PORT))

    print("Listening for broadcasted IMU data...")
    while True:
        data, _ = sock.recvfrom(1024)
        try:
            imu_data = json.loads(data.decode())
            print(f"Received Broadcast: {imu_data}")
        except json.JSONDecodeError:
            print("Error: Received malformed JSON data!")

if __name__ == "__main__":
    # Create and start the IMU simulator thread
    simulator_thread = threading.Thread(target=imu_simulator, daemon=True)
    simulator_thread.start()

    # Run the listener in the main thread
    listen_for_broadcast()
