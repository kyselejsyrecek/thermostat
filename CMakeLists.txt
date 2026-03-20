cmake_minimum_required(VERSION 3.16)

project(thermostat C)

set(CMAKE_C_STANDARD 99)

include(CheckIPOSupported)

set(PLATFORM linux CACHE STRING "linux esp32 stm32")

include_directories(
    lvgl
    lvgl/src
    lv_drivers
    app
)

file(GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS
    app/*.c
)

file(GLOB_RECURSE DRIVER_SRC
#    lv_drivers/*.c
#    lv_drivers/display/*.c
#    lv_drivers/touch/*.c
)

add_subdirectory(lvgl)

if(PLATFORM STREQUAL "linux")

    add_subdirectory(platform/linux)
    file(GLOB PLATFORM_SRC
        platform/linux/*.c
    )

elseif(PLATFORM STREQUAL "esp32")

    add_subdirectory(platform/esp32)
    file(GLOB PLATFORM_SRC
        platform/esp32/*.c
    )

elseif(PLATFORM STREQUAL "stm32")

    add_subdirectory(platform/stm32)
    file(GLOB PLATFORM_SRC
        platform/stm32/*.c
    )

else()

    message(FATAL_ERROR "Unknown platform")

endif()

add_executable(thermostat
    ${APP_SRC}
    ${DRIVER_SRC}
    ${PLATFORM_SRC}
)

target_link_libraries(thermostat
    lvgl
    ${PLATFORM_LIBRARIES}
)

# ── Per-platform compiler and linker flags ────────────────────────────────────
# Common to all platforms: LTO, dead-code elimination.
# Linux adds debug info (-ggdb) and size-with-debug optimisation (-Og).
# Embedded targets (ESP32, STM32) use full size optimisation (-Os).

# IPO/LTO — let CMake pick the right mechanism for the active compiler/linker
check_ipo_supported(RESULT _ipo_ok)
if(_ipo_ok)
    set_property(TARGET thermostat PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Dead-code elimination + per-platform optimisation level
set(_COMMON_COMPILE -ffunction-sections -fdata-sections)
set(_COMMON_LINK    -Wl,--gc-sections)

if(PLATFORM STREQUAL "linux")
    target_compile_options(thermostat PRIVATE -Og -ggdb ${_COMMON_COMPILE})
    target_link_options(thermostat    PRIVATE           ${_COMMON_LINK})
else()   # esp32, stm32
    target_compile_options(thermostat PRIVATE -Os        ${_COMMON_COMPILE})
    target_link_options(thermostat    PRIVATE           ${_COMMON_LINK})
endif()
