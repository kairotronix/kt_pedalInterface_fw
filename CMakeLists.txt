cmake_minimum_required(VERSION 3.13)

# Initialize from submodule
include(lib/pico_sdk/pico_sdk_init.cmake)

project(kt_pedalInterface_fw)

# Initialize the SDK
pico_sdk_init()

# Project-specifics
add_executable(kt_pedalInterface_fw src/main.c)
# Pull in common dependencies
target_link_libraries(kt_pedalInterface_fw pico_stdlib)
# Create map/bin/hex files
pico_add_extra_outputs(kt_pedalInterface_fw)