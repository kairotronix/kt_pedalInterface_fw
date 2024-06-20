cmake_minimum_required(VERSION 3.13)


# Initialize from submodule
include(lib/pico-sdk/pico_sdk_init.cmake)

project(kt_pedalInterface_fw C CXX ASM)

add_subdirectory(lib/pico_i2c_slave)

# Initialize the SDK
pico_sdk_init()

# Project-specifics
add_executable(kt_pedalInterface_fw src/main.c)
# Pull in common dependencies
target_link_libraries(kt_pedalInterface_fw pico_stdlib hardware_i2c hardware_irq hardware_gpio hardware_adc hardware_flash hardware_sync hardware_uart)
# Create map/bin/hex files
pico_add_extra_outputs(kt_pedalInterface_fw)