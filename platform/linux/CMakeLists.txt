cmake_minimum_required(VERSION 3.16)

option(PLATFORM_LINUX "Build for Linux" ON)
target_compile_definitions(lvgl PUBLIC PLATFORM_LINUX=1)

#add_subdirectory(../../lvgl lvgl_build)

find_package(SDL2 REQUIRED)
set_target_properties(SDL2::SDL2 PROPERTIES IMPORTED_GLOBAL TRUE)
set(PLATFORM_LIBRARIES SDL2::SDL2 PARENT_SCOPE)

include_directories(
    #../../app
    #../../drivers
    #../../lvgl
)
