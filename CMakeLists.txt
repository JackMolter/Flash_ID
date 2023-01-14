cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(flashOBC)

pico_sdk_init()

add_executable(flash
    flash.c
)

target_link_libraries(flash 
    pico_stdlib
    hardware_flash #
    hardware_dma
    hardware_irq
    hardware_spi
    
    )

pico_enable_stdio_usb(flash 1) # from video, enables USB for minicom 
pico_enable_stdio_uart(flash 0)

pico_add_extra_outputs(flash)