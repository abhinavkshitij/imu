default: run
.PHONY: sim

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Source files
SRC_DIR = ./src
SOURCES = ${SRC_DIR}/imu_parser.cpp

# Executable name
TARGET = imu_parser.exe

# Build 
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Run 
run: $(TARGET)
	./$(TARGET)

# Simulate
sim:
	python imu_sim.py

# Clean
clean:
	rm -f $(TARGET) 
