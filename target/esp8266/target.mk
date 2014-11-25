TARGET_OBJ_FILES := main.o \
					uart.o \
					interface_commands.o \
					info_commands.o \
					wifi_commands.o \
					ip_commands.o \
					ip_commands_info.o \
					ip_commands_common.o \
					ip_commands_socket.o \
					config_store.o \


TARGET_OBJ_PATHS := $(addprefix $(TARGET_DIR)/,$(TARGET_OBJ_FILES))

TOOLCHAIN_PREFIX ?= xtensa-lx106-elf-
XTENSA_TOOCHAIN := ../xtensa-lx106-elf/bin
CC := $(TOOLCHAIN_PREFIX)gcc
AR := $(TOOLCHAIN_PREFIX)ar
LD := $(TOOLCHAIN_PREFIX)gcc


XTENSA_LIBS ?= $(shell $(CC) -print-sysroot)

SDK_BASE ?= ../esp_iot_sdk_v0.9.3

SDK_AT_DIR := $(SDK_BASE)/examples/at

SDK_DRIVER_OBJ_FILES := 
SDK_DRIVER_OBJ_PATHS := $(addprefix $(SDK_AT_DIR)/driver/,$(SDK_DRIVER_OBJ_FILES))

CPPFLAGS += -I$(XTENSA_LIBS)/include \
			-I$(SDK_BASE)/include \
			-I$(SDK_AT_DIR)/include \
			-Itarget/esp8266/driver \
			-Itarget/esp8266

LDFLAGS  += -L$(XTENSA_LIBS)/lib \
			-L$(XTENSA_LIBS)/arch/lib \
			-L$(SDK_BASE)/lib

CFLAGS+=-std=c99
CPPFLAGS+=-DESP_PLATFORM=1

LIBS := c gcc hal phy net80211 lwip wpa main json ssl pp

#-Werror 
CFLAGS += -Os -g -O2 -Wpointer-arith -Wno-implicit-function-declaration -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mno-text-section-literals  -D__ets__ -DICACHE_FLASH

LDFLAGS	+= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

LD_SCRIPT := $(SDK_BASE)/ld/eagle.app.v6.ld

APP_AR:=$(BIN_DIR)/app.a
APP_OUT:=$(BIN_DIR)/app.elf
APP_FW_1 := $(BIN_DIR)/0x00000.bin
APP_FW_2 := $(BIN_DIR)/0x40000.bin
FULL_FW := $(BIN_DIR)/firmware.bin


$(APP_AR): $(COMMON_OBJ_PATHS) $(TARGET_OBJ_PATHS) $(SDK_DRIVER_OBJ_PATHS)
	$(AR) cru $@ $^

$(APP_AR): | $(BIN_DIR)

$(APP_OUT): $(APP_AR)
	$(LD) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(addprefix -l,$(LIBS)) $(APP_AR) -Wl,--end-group -o $@

firmware: $(APP_OUT)

all: firmware

clean-driver:
	rm -r $(SDK_DRIVER_OBJ_PATHS)

#clean:	clean-driver

.PHONY: all firmware
