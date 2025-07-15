# CMakeLists.txt for ESP32-S3 Knob Touch LCD Project

cmake_minimum_required(VERSION 3.16)

# Set the project name
set(PROJECT_NAME "esp32_s3_knob_lcd")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project
project(${PROJECT_NAME})
