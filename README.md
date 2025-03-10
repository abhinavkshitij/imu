# IMU parser 
1. An IMU outputs a byte stream at 921600 baud with a packet definition described below. Assuming some flavor of Linux running and data formatted according to IEEE-754 in network byte order, write the C++ code to parse data over a UART (`/dev/tty1`) and decommutate the byte stream into the four fields:
- Packet_Count
- X_Rate_rdps
- Y_Rate_rdps
- Z_Rate_rdps


## Packet description
|offset|size|type|desc|
|--|--|--|--|
|0|1|raw byte|start of frame byte - 7F|
|1|1|raw byte|start of frame byte - F0|
|2|1|raw byte|start of frame byte - 1C|
|3|1|raw byte|start of frame byte - AF|
|4|4|uint_t|pkt_count|
|8|4|float|x_gy_rate|
|12|4|float|y_gy_rate|
|16|4|float|z_gy_rate|

2. What modifications, if any, would you need to make to the code from 1. if the Linux host was a little-endian processor? Describe those modifications.

3. Write a C++ program that runs on Linux designed to execute in a resource constrained, multi-threaded environment that will execute the IMU parsing code every 80ms, and then broadcast the parsed results on the localhost network.

4. In Python, design a simple simulator/tester to drive your IMU parser. The simulator should send a string of bytes in the proper format over the UART for your parser to accept. The data contained in the byte string can be generated in any way of your choosing to best test the parser. Subsequently, it should then read the broadcasted results from 3. on the localhost network to validate the output.

## Build
Build and start packet processor (imu_parser). 
Then run IMU tester to print out broadcasted data.  
```sh
make
make sim
```
## Data and communication
- Stream data over UART at a given baud rate
    - try with `/dev/tty1`, if not available then `socat -d -d pty,raw,echo=0 pty,raw,echo=0`

- Broadcast data over network
    - Sockets API
    - port 5000

- Parse IMU telemetry data into python script
    - Listen on another thread to print IMU data

### Issues:
- Fix IEEE-753 format

