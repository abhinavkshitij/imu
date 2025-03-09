#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <arpa/inet.h>



#define UART_DEVICE "/dev/pts/6"  // Change to actual serial port
#define BAUDRATE B921600
#define PACKET_SIZE 20
#define SYNC_PATTERN "\x7F\xF0\x1C\xAF"

#pragma pack(push, 1)
struct IMUPacket {
    char sync[4];
    uint32_t packet_count;
    float x_rate;
    float y_rate;
    float z_rate;
};
#pragma pack(pop)

// Function to configure UART
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

// Function to read and parse IMU data
void read_imu_data(int fd) {
    IMUPacket imu_data;
    while (true) {
        int bytes_read = read(fd, &imu_data, PACKET_SIZE);
        if (bytes_read == PACKET_SIZE) {
            if (memcmp(imu_data.sync, SYNC_PATTERN, 4) == 0) {
                imu_data.packet_count = ntohl(imu_data.packet_count); // Convert to host byte order
                imu_data.x_rate = ntohf(imu_data.x_rate); // Convert to host byte order
                imu_data.y_rate = ntohf(imu_data.y_rate); // Convert to host byte order
                imu_data.z_rate = ntohf(imu_data.z_rate); // Convert to host byte order
                std::cout << "Received Packet: Count=" << imu_data.packet_count
                          << ", X=" <<  imu_data.x_rate
                          << ", Y=" << imu_data.y_rate
                          << ", Z=" << imu_data.z_rate << std::endl;
            } else {
                std::cerr << "Invalid sync pattern detected!" << std::endl;
            }
        }
    }
}

int main() {
    int serial_fd = configure_serial(UART_DEVICE);
    if (serial_fd == -1) return 1;

    std::cout << "Listening on " << UART_DEVICE << "..." << std::endl;
    read_imu_data(serial_fd);

    close(serial_fd);
    return 0;
}
