# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD pico_w CACHE STRING "Board type")


include(pico_sdk_import.cmake)

project(teste_2 CXX ASM)

pico_sdk_init()

add_executable(teste_2
    teste_2.c
)

pico_set_program_name(teste_2 "teste_2")
pico_set_program_version(teste_2 "0.1")


target_link_libraries(teste_2
    pico_stdlib
    hardware_spi
    hardware_dma
    fatfs
)

# Enable USB CDC (stdio over USB)
pico_enable_stdio_usb(teste_2 1)
pico_enable_stdio_uart(teste_2 0)

pico_add_extra_outputs(teste_2)
