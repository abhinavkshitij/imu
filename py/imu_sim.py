import serial
import socket
import struct
import time
import threading
import json

# Constants
UART_DEVICE = "/dev/pts/9"  # Change this based on the actual serial port
BAUDRATE = 921600
PACKET_SIZE = 20
SYNC_PATTERN = b"\x7F\xF0\x1C\xAF"
BROADCAST_PORT = 5000
INTERVAL_MS = 80

# Function to generate a test packet (in IEEE-754 format, network byte order)
def generate_test_packet(packet_count, x, y, z):
    return struct.pack("!4sIfff", SYNC_PATTERN, packet_count, x, y, z)

# Function to simulate IMU output (sends packets over UART)
def imu_simulator():
    with serial.Serial(UART_DEVICE, BAUDRATE, timeout=1) as ser:
        packet_count = 0
        while True:
            x, y, z = 1.23, -4.56, 7.89  # Example test values
            test_packet = generate_test_packet(packet_count, x, y, z)
            ser.write(test_packet)
            print(f"Sent Packet: Count={packet_count}, X={x}, Y={y}, Z={z}")
            packet_count += 1
            time.sleep(INTERVAL_MS / 1000.0)

# Function to listen for JSON broadcast over UDP
def listen_for_broadcast():
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
