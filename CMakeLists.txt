# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================



cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "D:/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(U1A5 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(lib/dht)

# Add executable. Default name is the project name, version 0.1

add_executable(U1A5 
    U1A5.c 
    lib/cyw43/cyw43.c
    lib/config/config.c
    lib/ssd1306/ssd1306.c
    lib/tasks/dht_task.c
    lib/tasks/ldr_task.c
    lib/tasks/display_task.c
    lib/tasks/mqtt_client_task.c
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
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
    )

# Add the standard include files to the build
target_include_directories(U1A5 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(U1A5 
        
        )

pico_add_extra_outputs(U1A5)

