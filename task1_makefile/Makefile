CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = build/main
SRC = main.cpp
TYPE ?= double
ifeq ($(TYPE), float)
    CXXFLAGS += -DUSE_FLOAT
endif
all: $(TARGET)
$(shell mkdir -p build)
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)