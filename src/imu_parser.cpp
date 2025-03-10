#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "imu_parser.h"


// Shared queue for inter-thread communication
std::queue<std::string> imu_data_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;

// Check system endianness
bool is_little_endian() {
    uint16_t test = 0x1;
    return (*((uint8_t*)&test) == 0x1);
}

// Convert IEEE-754 float from network byte order
float convert_float(uint32_t raw_value) {
    if (is_little_endian()) {
        raw_value = ntohl(raw_value);  // Swap byte order if needed
    }
    float value;
    std::memcpy(&value, &raw_value, sizeof(float));  // Reinterpret bytes as float
    return value;
}

// Configure UART
int configure_serial(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        std::cerr << "Error opening serial port: " << device << std::endl;
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

// Setup UDP socket for broadcasting
int setup_udp_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating UDP socket!" << std::endl;
        return -1;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error connecting UDP socket!" << std::endl;
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Function to broadcast data every 80ms
void broadcast_imu_data(int udp_sock) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(BROADCAST_INTERVAL_MS));

        std::unique_lock<std::mutex> lock(queue_mutex);
        if (!imu_data_queue.empty()) {
            std::string json_data = imu_data_queue.front();
            imu_data_queue.pop();
            lock.unlock();

            send(udp_sock, json_data.c_str(), json_data.length(), 0);
            std::cout << "Broadcasted: " << json_data << std::endl;
        } else {
            lock.unlock();
        }
    }
}

// Function to read IMU data from UART
void read_imu_data(int serial_fd) {
    IMUPacket imu_data;
    while (true) {
        int bytes_read = read(serial_fd, &imu_data, PACKET_SIZE);
        if (bytes_read == PACKET_SIZE) {
            if (memcmp(imu_data.sync, SYNC_PATTERN, 4) == 0) {
                uint32_t packet_count = ntohl(imu_data.packet_count);
                float x_rate = convert_float(imu_data.x_raw);
                float y_rate = convert_float(imu_data.y_raw);
                float z_rate = convert_float(imu_data.z_raw);

                std::ostringstream json_stream;
                json_stream << "{"
                            << "\"packet_count\":" << packet_count << ","
                            << "\"x\":" << x_rate << ","
                            << "\"y\":" << y_rate << ","
                            << "\"z\":" << z_rate
                            << "}";

                std::string json_data = json_stream.str();

                std::cout << "Received Packet: Count=" << packet_count
                          << ", X=" << x_rate
                          << ", Y=" << y_rate
                          << ", Z=" << z_rate << std::endl;

                // Store in queue for broadcasting
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    imu_data_queue.push(json_data);
                }
            } else {
                std::cerr << "Invalid sync pattern detected!" << std::endl;
            }
        }
    }
}

int main() {
    int serial_fd = configure_serial(UART_DEVICE);
    if (serial_fd == -1) return 1;

    int udp_sock = setup_udp_socket();
    if (udp_sock == -1) return 1;

    std::cout << "Listening on " << UART_DEVICE << " and broadcasting to " 
              << BROADCAST_IP << ":" << BROADCAST_PORT << " every 80ms..." << std::endl;

    // Create threads for reading UART and broadcasting data every 80ms
    std::thread read_thread(read_imu_data, serial_fd);
    std::thread broadcast_thread(broadcast_imu_data, udp_sock);

    read_thread.join();
    broadcast_thread.join();

    close(serial_fd);
    close(udp_sock);
    return 0;
}
