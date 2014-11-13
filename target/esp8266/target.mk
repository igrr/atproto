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

XTENSA_TOOCHAIN := ../xtensa-lx106-elf/bin
CC := $(XTENSA_TOOCHAIN)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOCHAIN)/xtensa-lx106-elf-ar
LD := $(XTENSA_TOOCHAIN)/xtensa-lx106-elf-gcc

XTENSA_LIBS := ../RC-2010.1-linux/lx106/xtensa-elf

ESPTOOL := ../esptool/esptool

SDK_BASE := ../esp_iot_sdk_v0.9.2

SDK_AT_DIR := $(SDK_BASE)/examples/at

SDK_DRIVER_OBJ_FILES := 
SDK_DRIVER_OBJ_PATHS := $(addprefix $(SDK_AT_DIR)/driver/,$(SDK_DRIVER_OBJ_FILES))

CPPFLAGS += -I$(XTENSA_LIBS)/include \
			-I$(SDK_BASE)/include \
			-I$(SDK_AT_DIR)/include

LDFLAGS  += -L$(XTENSA_LIBS)/lib \
			-L$(XTENSA_LIBS)/arch/lib \
			-L$(SDK_BASE)/lib

CFLAGS+=-std=c99
CPPFLAGS+=-DESP_PLATFORM=1

LIBS := c gcc hal phy net80211 lwip wpa main json ssl upgrade upgrade_ssl

#-Werror 
CFLAGS += -Os -g -O2 -Wpointer-arith -Wno-implicit-function-declaration -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mno-text-section-literals  -D__ets__ -DICACHE_FLASH

LDFLAGS	+= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

LD_SCRIPT := $(SDK_BASE)/ld/eagle.app.v6.ld

APP_AR:=$(BIN_DIR)/app.a
APP_OUT:=$(BIN_DIR)/app.out
APP_FW_1 := $(BIN_DIR)/0x00000.bin
APP_FW_2 := $(BIN_DIR)/0x40000.bin
FULL_FW := $(BIN_DIR)/firmware.bin


$(APP_AR): $(COMMON_OBJ_PATHS) $(TARGET_OBJ_PATHS) $(SDK_DRIVER_OBJ_PATHS)
	$(AR) cru $@ $^

$(APP_AR): | $(BIN_DIR)

$(APP_OUT): $(APP_AR)
	$(LD) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(addprefix -l,$(LIBS)) $(APP_AR) -Wl,--end-group -o $@

$(APP_FW_1): $(APP_OUT)
	$(ESPTOOL) -eo $(APP_OUT) -bo $@ -bs .text -bs .data -bs .rodata -bc -ec

$(APP_FW_2): $(APP_OUT)
	$(ESPTOOL) -eo $(APP_OUT) -es .irom0.text $@ -ec

$(FULL_FW): $(APP_FW_1) $(APP_FW_2)
	dd if=/dev/zero ibs=4k count=124 | LC_ALL=C tr "\000" "\377" >$(FULL_FW)
	dd if=$(APP_FW_1) of=$(FULL_FW) bs=4k seek=0 conv=notrunc
	dd if=$(APP_FW_2) of=$(FULL_FW) bs=4k seek=64 conv=notrunc

firmware: $(APP_FW_1) $(APP_FW_2) $(FULL_FW)

all: firmware

clean-driver:
	rm -r $(SDK_DRIVER_OBJ_PATHS)

#clean:	clean-driver

.PHONY: all firmware
