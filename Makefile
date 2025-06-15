DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
ESP_ROOT = ${DIR}/thirdParty/arduino-esp32/
CHIP = esp32
BOARD = esp32wrover
SKETCH = $(DIR)/src/hla_main.cpp
BUILD_DIR = $(DIR)/build
LIBS =	\
		${DIR}/include \
		${DIR}/src/* \
		${DIR}/thirdParty/ArduinoJson/src \
		${DIR}/thirdParty/base64_arduino \
		${DIR}/thirdParty/AsyncTCP \
		${DIR}/thirdParty/ESPAsyncWebServer/src \
		${DIR}/thirdParty/arduino-esp32/libraries/ESPmDNS/src/
FS_TYPE = littlefs
FS_DIR = ${DIR}/data

include $(DIR)/thirdParty/makeEspArduino/makeEspArduino.mk