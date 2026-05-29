# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

# Directories
SRC_DIR = src
BIN_DIR = bin

# Files
TARGET = $(BIN_DIR)/raycast
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Note: Headers (.h) are not compiled directly, but we track them for dependencies if needed.
# For a simple build, tracking changes in .c files is usually sufficient.
TEST_SCRIPT := test/run_tests.py

# Default rule: Build the target
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Rule to run the program (similar to your previous command)
# run: $(TARGET)
# 	./$(TARGET)

# Rule to run tests
check: all
	$(TEST_SCRIPT)

# Rule to clean up build artifacts
clean:
	rm -f $(TARGET)
	rm -rf $(BIN_DIR)

.PHONY: all clean

extract_data.tgz:
	@make -C test > /dev/null

all: extract_data.tgz

