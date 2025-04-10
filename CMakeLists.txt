# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "$ENV{PROJECT_DIR}/src/app/main"
)

set(COMPONENT_DIRS 
    "$ENV{IDF_PATH}/components"
    "$ENV{PROJECT_DIR}/src/app"
    "$ENV{PROJECT_DIR}/src/app/device_drivers"
    "$ENV{EXTRA_COMPONENT_DIRS}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(SCAN)
