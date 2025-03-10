#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <arpa/inet.h>

#define UART_DEVICE "/dev/pts/10"  // Change to actual serial port
#define BAUDRATE B921600
#define PACKET_SIZE 20
#define SYNC_PATTERN "\x7F\xF0\x1C\xAF"

#pragma pack(push, 1)
struct IMUPacket {
    char sync[4];
    uint32_t packet_count;
    uint32_t x_raw;
    uint32_t y_raw;
    uint32_t z_raw;
};
#pragma pack(pop)

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
                uint32_t packet_count = ntohl(imu_data.packet_count);
                float x_rate = convert_float(imu_data.x_raw);
                float y_rate = convert_float(imu_data.y_raw);
                float z_rate = convert_float(imu_data.z_raw);

                std::cout << "Received Packet: Count=" << packet_count
                          << ", X=" << x_rate
                          << ", Y=" << y_rate
                          << ", Z=" << z_rate << std::endl;
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
