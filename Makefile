PROGRAM = main

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/rboot-ota \
	extras/dhcpserver \
	extras/pwm \
	$(abspath ../../external_libs/wolfssl) \
        $(abspath ../../external_libs/cJSON) \
        $(abspath ../../external_libs/homekit) \
        $(abspath ../../external_libs/wifi_config) \

FLASH_SIZE ?= 8
FLASH_MODE ?= dout
FLASH_SPEED ?= 40

HOMEKIT_SPI_FLASH_BASE_ADDR = 0x8c000
HOMEKIT_MAX_CLIENTS = 16
HOMEKIT_SMALL = 0
HOMEKIT_OVERCLOCK ?= 1
HOMEKIT_OVERCLOCK_PAIR_SETUP ?= 1
HOMEKIT_OVERCLOCK_PAIR_VERIFY ?= 1
EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS -DWIFI_CONFIG_CONNECT_TIMEOUT=180000

include $(abspath ../../sdk/esp-open-rtos/common.mk)

LIBS += m

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
