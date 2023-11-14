#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
PROJECT_NAME := smart_switch

EXTRA_COMPONENT_DIRS := $(realpath ../../esp-ali-smartliving)

# export IDF_TARGET = esp8266

SDKCONFIG_DEFAULTS := ./sdkconfig_defaults/sdkconfig.defaults.$(IDF_TARGET)

include $(IDF_PATH)/make/project.mk