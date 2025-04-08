CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -lraylib

EXE = spotlightpp
SRC_DIR = ./src
BUILD_DIR = ./build

CXX_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CXX_OBJS := $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
C_OBJS := $(C_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/$(EXE)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@

$(BUILD_DIR)/$(EXE): $(CXX_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) $(CXX_OBJS) $(C_OBJS) -o $@

clean:
	rm -rf $(BUILD_DIR)/*

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

install: all
	sudo strip -o /usr/bin/spotlightpp $(abspath $(BUILD_DIR)/$(EXE))

.PHONY: all clean
