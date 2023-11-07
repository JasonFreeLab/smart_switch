set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "linkkit_solo.c")

set(COMPONENT_REQUIRES "esp-ali-smartliving" "json" "switch" "led")

register_component()
