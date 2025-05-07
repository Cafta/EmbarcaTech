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
# =====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Desafio_01_Galton_Board C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Desafio_01_Galton_Board Desafio_01_Galton_Board.c )

pico_set_program_name(Desafio_01_Galton_Board "Desafio_01_Galton_Board")
pico_set_program_version(Desafio_01_Galton_Board "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Desafio_01_Galton_Board 0)
pico_enable_stdio_usb(Desafio_01_Galton_Board 1)

# Add the standard library to the build
target_link_libraries(Desafio_01_Galton_Board
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Desafio_01_Galton_Board PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Caminho para o .c da lib OLED
target_sources(Desafio_01_Galton_Board PRIVATE
    inc/ssd1306_i2c.c
)

# Add any user requested libraries
target_link_libraries(Desafio_01_Galton_Board 
        hardware_i2c
        pico_multicore
        )

pico_add_extra_outputs(Desafio_01_Galton_Board)

