CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS ?= -Iinclude

TARGET := bin/nvme_gpu_lab
SOURCES := src/main.cpp src/file_reader.cpp src/result.cpp src/size_parser.cpp
OBJECTS := $(SOURCES:src/%.cpp=build/%.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build bin:
	mkdir -p $@

test: $(TARGET)
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -rf build bin data/test-smoke.bin
