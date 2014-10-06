TARGET_OBJ_FILES := dce_test_commands.o \
					test_dce_utils.o \
					tests.o

TARGET_OBJ_PATHS := $(addprefix $(TARGET_DIR)/,$(TARGET_OBJ_FILES))

BINARY_NAME := $(BIN_DIR)/dce_test

all: $(BINARY_NAME)

$(BINARY_NAME): $(COMMON_OBJ_PATHS) $(TARGET_OBJ_PATHS)
	g++ $(LD_FLAGS) -o $@ $^

$(BINARY_NAME): | $(BIN_DIR)

test: all
	$(BINARY_NAME)

.PHONY: all test
