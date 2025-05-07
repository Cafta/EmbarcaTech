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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(U7T_CFMA C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(U7T_CFMA U7T_CFMA.c)

pico_set_program_name(U7T_CFMA "U7T_CFMA")
pico_set_program_version(U7T_CFMA "1.2")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(U7T_CFMA 0)
pico_enable_stdio_usb(U7T_CFMA 1)

# Add the standard library to the build
target_link_libraries(U7T_CFMA
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(U7T_CFMA PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(U7T_CFMA 
        pico_stdlib
        hardware_i2c
        hardware_clocks
        hardware_pwm
        hardware_adc
        hardware_pio
        )

target_sources(U7T_CFMA PRIVATE 
    inc/ssd1306_i2c.c  # 🛠️ Caminho correto do arquivo .c
)

# Gera o .pio.h
pico_generate_pio_header(U7T_CFMA ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Define os binários finais
pico_add_extra_outputs(U7T_CFMA)

