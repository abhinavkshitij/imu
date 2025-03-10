// imu_constants.h
#ifndef IMU_CONSTANTS_H
#define IMU_CONSTANTS_H

#define UART_DEVICE "/dev/pts/10"  // Change to actual serial port
#define BAUDRATE B921600
#define PACKET_SIZE 20
#define SYNC_PATTERN "\x7F\xF0\x1C\xAF"
#define BROADCAST_IP "127.0.0.1"
#define BROADCAST_PORT 5000
#define BROADCAST_INTERVAL_MS 80  // 80ms interval

#endif // IMU_CONSTANTS_H