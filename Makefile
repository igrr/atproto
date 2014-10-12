TARGET ?= host
TARGET_DIR := target/$(TARGET)
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CPPFLAGS+=-Iinclude -Isrc/include 
CFLAGS += -std=c99
CXXFLAGS += -std=c++11

COMMON_OBJ_FILES := dce.o \
					dce_basic_commands.o \
					dce_utils.o \
					

COMMON_OBJ_PATHS := $(addprefix $(SRC_DIR)/,$(COMMON_OBJ_FILES))

include $(TARGET_DIR)/target.mk

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(TARGET_DIR)/*.o
	rm -rf $(BIN_DIR)

.PHONY: clean

