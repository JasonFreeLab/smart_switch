set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "switch.c")

idf_build_get_property(target IDF_TARGET)
if(${target} STREQUAL esp8266)
set(COMPONENT_REQUIRES )
else()
set(COMPONENT_REQUIRES "esp_driver_gpio")
endif()

register_component()
