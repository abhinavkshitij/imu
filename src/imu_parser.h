// imu_parser.h
#ifndef IMU_PARSER_H
#define IMU_PARSER_H

#include "imu_constants.h"
#include <cstdint>
#include <netinet/in.h>

#pragma pack(push, 1)

struct IMUPacket {
    char sync[4];
    uint32_t packet_count;
    uint32_t x_raw;
    uint32_t y_raw;
    uint32_t z_raw;
};
#pragma pack(pop)

bool is_little_endian();
float convert_float(uint32_t raw_value);
int configure_serial(const char* device);
int setup_udp_socket();
void broadcast_imu_data(int udp_sock);
void read_imu_data(int serial_fd);

#endif // IMU_PARSER_H