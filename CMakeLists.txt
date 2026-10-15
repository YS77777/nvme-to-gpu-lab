cmake_minimum_required(VERSION 3.20)

project(nvme_to_gpu_lab VERSION 0.1.0 LANGUAGES CXX)

option(ENABLE_CUDA "Build the CUDA backends introduced in stage 2" OFF)

if(ENABLE_CUDA)
  message(FATAL_ERROR
    "ENABLE_CUDA belongs to stage 2 and is not implemented in v0.1 yet. "
    "Configure with -DENABLE_CUDA=OFF.")
endif()

add_executable(nvme_gpu_lab
  src/main.cpp
  src/file_reader.cpp
  src/result.cpp
  src/size_parser.cpp
)

target_include_directories(nvme_gpu_lab PRIVATE include)
target_compile_features(nvme_gpu_lab PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(nvme_gpu_lab PRIVATE /W4 /permissive-)
else()
  target_compile_options(nvme_gpu_lab PRIVATE -Wall -Wextra -Wpedantic)
endif()
