// imu_parser.cpp
#include "imu_parser.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>


// TODO: Finx IEEE-753 conversion
// Convert float to little endian
std::uint32_t ntohf(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(value));

    // Check if the system is big-endian
    int n = 1;
    if (*(char *)&n == 0) {
        // Reverse the byte order for big-endian systems
        std::uint8_t *bytes = reinterpret_cast<std::uint8_t*>(&result);
        std::reverse(bytes, bytes + sizeof(result));
    }
    return result;
}


// Function to configure UART
int init_uart() {
    int uart_fd = open(UART_DEVICE, O_RDONLY | O_NOCTTY);
    if (uart_fd < 0) {
        std::cerr << "Failed to open UART device!" << std::endl;
        return -1;
    }

    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    tcsetattr(uart_fd, TCSANOW, &options);
    return uart_fd;
}

// Function to read and parse IMU data
bool read_imu_data(int uart_fd, IMUPacket &packet) {
    uint8_t buffer[PACKET_SIZE];
    uint8_t sync[4];
    
    while (read(uart_fd, sync, 4) == 4) {
        if (memcmp(sync, SYNC_PATTERN, 4) == 0) {
            if (read(uart_fd, buffer, PACKET_SIZE - 4) == PACKET_SIZE - 4) {
                std::memcpy(&packet.packet_count, buffer, 4);
                std::memcpy(&packet.x_rate_rdps, buffer + 4, 4);
                std::memcpy(&packet.y_rate_rdps, buffer + 8, 4);
                std::memcpy(&packet.z_rate_rdps, buffer + 12, 4);
                
                packet.packet_count = ntohl(packet.packet_count);
                packet.x_rate_rdps = ntohf(packet.x_rate_rdps);
                packet.y_rate_rdps = ntohf(packet.y_rate_rdps);
                packet.z_rate_rdps = ntohf(packet.z_rate_rdps);
                return true;
            }
        }
    }
    return false;
}

int init_udp_socket(struct sockaddr_in &broadcast_addr) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket!" << std::endl;
        return -1;
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
   
    return sock;
}

void imu_reader() {
    int uart_fd = init_uart();
    if (uart_fd < 0) return;

    struct sockaddr_in broadcast_addr;
    int sock = init_udp_socket(broadcast_addr);
    if (sock < 0) {
        close(uart_fd);
        return;
    }

    IMUPacket packet;
    while (true) {
        if (read_imu_data(uart_fd, packet)) {
            sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_MS));
    }
    close(sock);
    close(uart_fd);
}

int main() {
    int serial_fd = init_uart();
    if (serial_fd == -1) return 1;

    std::cout << "Listening on " << UART_DEVICE << "..." << std::endl;
    std::thread imu_thread(imu_reader);
    imu_thread.join();
    return 0;
}
