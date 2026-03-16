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

# LTO (link-time optimization) — works with GCC, Clang and cross-compilers
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set_property(TARGET thermostat PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Dead-code elimination — remove unused functions and data
target_compile_options(thermostat PRIVATE -ffunction-sections -fdata-sections)
target_link_options(thermostat PRIVATE -Wl,--gc-sections)
