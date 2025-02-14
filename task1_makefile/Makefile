CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = build/main
SRC = main.cpp
all: $(TARGET)
$(shell mkdir -p build)
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)