SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
BINARY_NAME := $(BIN_DIR)/dce_test

C_FILES := $(wildcard $(SRC_DIR)/*.c)
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
C_OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(C_FILES:.c=.o)))
CPP_OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))

#LD_FLAGS += 
#CC_FLAGS += 
CPPFLAGS+=-Iinclude
all: $(BINARY_NAME)

$(BINARY_NAME): $(CPP_OBJ_FILES) $(C_OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^

$(CPP_OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) 
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(C_OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) 
	gcc $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BINARY_NAME): | $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

test: all
	$(BINARY_NAME)

.PHONY: clean all test
