SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
BINARY_NAME := $(BIN_DIR)/dce_test

TARGET := host

COMMON_OBJ_FILES := $(SRC_DIR)/dce.o \
					$(SRC_DIR)/dce_basic_commands.o \
					$(SRC_DIR)/dce_utils.o \
					$(SRC_DIR)/dce_info_commands.o

TARGET_DIR := target/$(TARGET)

TARGET_OBJ_FILES := $(TARGET_DIR)/dce_test_commands.o \
					$(TARGET_DIR)/test_dce_utils.o \
					$(TARGET_DIR)/tests.o


CPPFLAGS+=-Iinclude -Isrc/include

all: $(BINARY_NAME)

$(BINARY_NAME): $(COMMON_OBJ_FILES) $(TARGET_OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

# $(CPP_OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) 
# 	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

# $(C_OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) 
# 	gcc $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BINARY_NAME): | $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm $(SRC_DIR)/*.o
	rm $(TARGET_DIR)/*.o
	rm -rf $(BIN_DIR)

test: all
	$(BINARY_NAME)

.PHONY: clean all test
