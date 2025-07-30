cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Pico-FreeRTOS C CXX ASM)

pico_sdk_init()

add_subdirectory(lib)
add_subdirectory(src)

set(SRC_FILES
    src/main.c
    src/led.c
    src/cpu_monitor.c
    src/adc_custom.c
    src/Bluethooth.c
    src/i2c.c
    src/oled.c
    src/fonts.c
    src/dht11.c
    src/wifi.c
    src/servo_motor.c
    src/Timer.c
)

target_include_directories(app PUBLIC inc)