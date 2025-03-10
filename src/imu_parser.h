// imu_parser.h
#ifndef IMU_PARSER_H
#define IMU_PARSER_H

#include "imu_constants.h"
#include <cstdint>
#include <netinet/in.h>

#pragma pack(push, 1)
struct IMUPacket {
    uint32_t packet_count;
    float x_rate_rdps;
    float y_rate_rdps;
    float z_rate_rdps;
};
#pragma pack(pop)

uint32_t ntohf(float);
int init_uart();
int init_udp_socket(struct sockaddr_in &broadcast_addr);
bool read_imu_data(int uart_fd, IMUPacket &packet);
void imu_reader();

#endif // IMU_PARSER_H