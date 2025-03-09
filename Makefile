default: all

.PHONY:
    all: imu
    test: packet

imu:
	gcc -o imu imu.cpp

imu_test:
	python test_imu.py