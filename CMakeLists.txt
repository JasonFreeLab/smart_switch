
# (Automatically converted from project Makefile by convert_to_cmake.py.)

# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(__PROJECT_GROUP_LINK_COMPONENTS 1)

set(EXTRA_COMPONENT_DIRS "../../esp-ali-smartliving")

# set(IDF_TARGET "esp8266")

set(SDKCONFIG_DEFAULTS ${CMAKE_CURRENT_LIST_DIR}/sdkconfig_defaults/sdkconfig.defaults.$ENV{IDF_TARGET})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smart_switch)
