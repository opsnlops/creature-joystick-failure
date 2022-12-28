cmake_minimum_required(VERSION 3.20)

# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Our source files
file(GLOB sourceFiles
        src/*
        src/joystick/*
        src/logging/*
        )

add_executable(joystick)

target_sources(joystick PUBLIC
        ${sourceFiles}
        )

target_include_directories(joystick PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        .
        )

add_subdirectory(pico-ssd1306)

# pull in common dependencies
target_link_libraries(joystick PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        tinyusb_board
        tinyusb_device
        hardware_adc
        hardware_gpio
        hardware_uart
        pico_stdlib
        pico_ssd1306
        )

# Enable the console over UART (which is the picoprobe!)
pico_enable_stdio_usb(joystick 0)
pico_enable_stdio_uart(joystick 1)

# Create all of the files
pico_add_extra_outputs(joystick)
