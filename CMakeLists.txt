


cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(U1A5 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(lib/dht)

# Add executable. Default name is the project name, version 0.1

add_executable(U1A5 
    U1A5.c 
    lib/cyw43/cyw43.c 
    lib/mqtt/mqtt.c
    lib/config/config.c
    lib/ssd1306/ssd1306.c
)

pico_set_program_name(U1A5 "U1A5")
pico_set_program_version(U1A5 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(U1A5 0)
pico_enable_stdio_usb(U1A5 1)

# Add the standard library to the build
target_link_libraries(U1A5
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pwm
    dht
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
    )

# Add the standard include files to the build
target_include_directories(U1A5 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(U1A5 
        
        )

pico_add_extra_outputs(U1A5)

