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

# Single source of truth for the firmware version (src/version.h).
# CMake parses the macros so the build artifact and pico_set_program_version
# stay in lockstep with what the firmware reports at runtime.
file(READ "${CMAKE_CURRENT_LIST_DIR}/src/version.h" _version_h)
string(REGEX MATCH "TOUCHORD_VERSION_MAJOR ([0-9]+)" _ "${_version_h}")
set(TOUCHORD_VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "TOUCHORD_VERSION_MINOR ([0-9]+)" _ "${_version_h}")
set(TOUCHORD_VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "TOUCHORD_VERSION_PATCH ([0-9]+)" _ "${_version_h}")
set(TOUCHORD_VERSION_PATCH "${CMAKE_MATCH_1}")
set(TOUCHORD_VERSION "${TOUCHORD_VERSION_MAJOR}.${TOUCHORD_VERSION_MINOR}.${TOUCHORD_VERSION_PATCH}")

project(Touchord
    VERSION ${TOUCHORD_VERSION}
    LANGUAGES C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

set(TOUCHORD_SOURCES
    src/Touchord.c src/Globals.c src/Sync.c src/usb_descriptors.c
    src/Modes/Compose.c src/Modes/Strum.c src/Modes/Settings.c
    src/Modes/Perform.c src/Modes/Omni.c src/Modes/Drum.c
    src/Notes/Note.c
    src/Rendering/Graphics.c src/Rendering/ScreenDma.c
    src/IO/Midi.c src/IO/Trill.c
    src/Data/Flash.c
    src/thirdparty/Fonts.c src/thirdparty/ssd1306.c
)

# Two firmware variants share the same source list and only differ in the
# TOUCHORD_DUAL_I2C compile definition. Both build by default so users with
# either hardware revision can grab the matching .uf2 from build/.
function(touchord_add_variant target dual_i2c)
    add_executable(${target} ${TOUCHORD_SOURCES})

    pico_set_program_name(${target} "${target}")
    pico_set_program_version(${target} "${TOUCHORD_VERSION}")

    pico_enable_stdio_uart(${target} 1)
    pico_enable_stdio_usb(${target} 1)

    target_link_libraries(${target}
        pico_stdlib
        pico_multicore
        hardware_flash
        hardware_sync
        hardware_spi
        hardware_i2c
        hardware_dma
        tinyusb_device
        tinyusb_board)

    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/thirdparty)

    target_compile_definitions(${target} PRIVATE TOUCHORD_DUAL_I2C=${dual_i2c})

    pico_add_extra_outputs(${target})
endfunction()

touchord_add_variant(Touchord-dual   1)  # Trill on i2c0 (GPIO16/17), screen on i2c1 (GPIO14/15)
touchord_add_variant(Touchord-single 0)  # Trill + screen share i2c0 (GPIO16/17)

