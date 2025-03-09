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

2. What modifications, if any, would you need to make to the code from 1. if the Linux host was a little-
endian processor? Describe those modifications.
    - lsb comes before msb
    - count bottom to top in the memory map

3. Write a C++ program that runs on Linux designed to execute in a resource constrained, multi-threaded
environment that will execute the IMU parsing code every 80ms, and then broadcast the parsed results
on the localhost network.

4. In Python, design a simple simulator/tester to drive your IMU parser. The simulator should send a string
of bytes in the proper format over the UART for your parser to accept. The data contained in the byte
string can be generated in any way of your choosing to best test the parser. Subsequently, it should then
read the broadcasted results from 3) on the localhost network to validate the output.

## Data and communication
- Network uses big-endianess (msb first)
- Linux uses little-endiness (lsb first)

- Baud rate, sampling rate (ms) conversion 
- Read IMU data from read-only registers 

- Stream data over UART at a given baud rate
    - display in tty

- Broadcast data over network
    - Sockets API?

- Parse IMU telemetry data into python script
    - Build .py parser
    - Create network (localhost, loopback)
    - ports?


