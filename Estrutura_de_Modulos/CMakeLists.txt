# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w CACHE STRING "Board type")

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

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Estrutura_de_Modulos C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

add_executable(Estrutura_de_Modulos 
    app/main.c
    hal/hal_led.c
    drivers/led_embutido.c
)

pico_set_program_name(Estrutura_de_Modulos "Estrutura_de_Modulos")
pico_set_program_version(Estrutura_de_Modulos "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Estrutura_de_Modulos 0)
pico_enable_stdio_usb(Estrutura_de_Modulos 1)

# Add the standard library to the build
target_link_libraries(Estrutura_de_Modulos
    pico_stdlib
    pico_cyw43_arch_none
)

# Add the standard include files to the build
target_include_directories(Estrutura_de_Modulos PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Add any user requested libraries
target_link_libraries(Estrutura_de_Modulos 
        
        )

pico_add_extra_outputs(Estrutura_de_Modulos)

