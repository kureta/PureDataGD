# Define variables for directories
SOURCE_DIR := .
BUILD_DIR := build

# Default target
all: configure build

# Configure the project using CMake
configure:
	mkdir -p $(BUILD_DIR)
	cmake -S $(SOURCE_DIR) -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
build: configure
	$(MAKE) -j8 -C $(BUILD_DIR)

install: build
	cp $(BUILD_DIR)/libPureDataGD.so ./demo/bin/

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)

# Remove compile_commands.json
clean-commands:
	rm -f $(BUILD_DIR)/compile_commands.json

# Phony targets to avoid conflict with file names
.PHONY: all configure build clean clean-commands
