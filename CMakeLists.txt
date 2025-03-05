cmake_minimum_required(VERSION 3.10)
project(ActuatorsController)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create an executable for Mega2560 using mega2560_main.cpp
add_executable(mega2560 src/mega2560_main.cpp)
target_include_directories(mega2560 PRIVATE "C:/Users/fredr/Documents/Arduino/libraries")

# Create an executable for ESP32 using esp32_main.cpp
add_executable(esp32 src/esp32_main.cpp)
target_include_directories(esp32 PRIVATE "C:/Users/fredr/Documents/Arduino/libraries")