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


## Dev notes
Clone the project from github repo `https://github.com/abhinavkshitij/imu.git` and checkout to `develop` branch. The project dev follows Gitflow, so feel free to create addl branches as required. ideally the `main` should be protected, while `develop` may be protected additionally. 

Future development scope includes:
- Test suite for python and cpp stubs. Developer should add unit test and integration tests during developement. 
- CI pipeline can be setup as Github Actions or any other CI tool (Jenkins, GitlabCI, Bamboo)
- Makefile should suffice build and run for this project, but CMake and CTest is preferred over Makefile for cross-compilation
- Replace SIL runs in an RTOS environment to accurately determine the order of operation
- Transform stubs into reusable components. Then publish the pacakges (conan for CPP and pypi for python) to an artifactory location. 
- Add security measures. Packages must be pulled from artifactory after being scanned in a DevOps pipeline.    

## Build notes
Build and start packet processor (imu_parser). 
Then run IMU tester to print out broadcasted data.  
```sh
cd {PROJECT_ROOT}
python -m venv .venv
source ./.venv/bin/activate
pip install -r requirements.txt
make
make sim
```
After running the project, you may want to cleanup
```sh
make clean
deactivate
```

## Issues and fixes:
- Fix IEEE-754 format

Test scripts in `py/stubs/` send and receive data in the correct order. 
Require CPP function to replicate the same behavior.   

### Data and communication
- Stream data over UART at a given baud rate
    - try with `/dev/tty1`
    - if not available then `socat -d -d pty,raw,echo=0 pty,raw,echo=0`

- Broadcast data over a local network
    - localhost:5000

- Parse IMU telemetry data into python script
    - Listen on another thread to print IMU data

```cpp
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
```