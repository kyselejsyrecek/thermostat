cmake_minimum_required(VERSION 3.16)

# TODO Stub.

set(IDF_PATH ../../vendor/esp-idf)

execute_process(
    COMMAND ${IDF_PATH}/tools/idf.py build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#idf_component_register(
#    SRCS
#        main.c
#        ../../app/gui/ui.c
#        ../../../app/app.c
#    INCLUDE_DIRS
#        .
#        ../../../app
#        ../../../lvgl
#)
