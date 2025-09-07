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
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Touchord C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Touchord src/Touchord.c src/Midi.c src/Globals.c src/Fonts.c src/usb_descriptors.c src/ssd1306.c
                        src/Modes/Compose.c src/Modes/Strum.c src/Modes/Settings.c src/Modes/Perform.c src/Modes/Omni.c
                        src/Notes/Note.c)

pico_set_program_name(Touchord "Touchord")
pico_set_program_version(Touchord "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Touchord 1)
pico_enable_stdio_usb(Touchord 0)

# Add the standard library to the build
target_link_libraries(Touchord
        pico_stdlib
        pico_multicore
        tinyusb_device
        tinyusb_board)

# Add the standard include files to the build
target_include_directories(Touchord PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)

# Add any user requested libraries
target_link_libraries(Touchord 
        hardware_spi
        hardware_i2c
        
        )

pico_add_extra_outputs(Touchord)

