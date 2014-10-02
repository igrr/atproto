SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin


TARGET := host

TARGET_DIR := target/$(TARGET)


COMMON_OBJ_FILES := $(SRC_DIR)/dce.o \
					$(SRC_DIR)/dce_basic_commands.o \
					$(SRC_DIR)/dce_utils.o \
					$(SRC_DIR)/dce_info_commands.o

ifeq ($(TARGET),host)
TARGET_OBJ_FILES := $(TARGET_DIR)/dce_test_commands.o \
					$(TARGET_DIR)/test_dce_utils.o \
					$(TARGET_DIR)/tests.o

CPPFLAGS+=-Iinclude -Isrc/include

BINARY_NAME := $(BIN_DIR)/dce_test
all: $(BINARY_NAME)

$(BINARY_NAME): $(COMMON_OBJ_FILES) $(TARGET_OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

$(BINARY_NAME): | $(BIN_DIR)

test: all
	$(BINARY_NAME)

else ifeq ($(TARGET),esp8266)

CPPFLAGS += -D__STDC_HOSTED__=0

TARGET_OBJ_FILES := $(TARGET_DIR)/main.o
ESP_TOOCHAIN := ../xtensa-lx106-elf/bin
CC := $(ESP_TOOCHAIN)/xtensa-lx106-elf-gcc
AR := $(ESP_TOOCHAIN)/xtensa-lx106-elf-ar
LD := $(ESP_TOOCHAIN)/xtensa-lx106-elf-gcc

all: $(COMMON_OBJ_FILES)

endif


$(BIN_DIR):
	mkdir -p $(BIN_DIR)


clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(TARGET_DIR)/*.o
	rm -rf $(BIN_DIR)

.PHONY: clean all test
