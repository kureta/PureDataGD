# Define variables for directories
SOURCE_DIR := .
BUILD_DIR := build
OUTPUT_ARTIFACT := libPureDataGD.so
# Use shell command to find all source files
SRC_FILES := $(shell find $(SOURCE_DIR) -name '*.cpp' -o -name '*.h')

# Default target
all: configure build install

# Configure the project using CMake
configure: $(BUILD_DIR)/Makefile
$(BUILD_DIR)/Makefile: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake -S $(SOURCE_DIR) -B $(BUILD_DIR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_BUILD_TYPE=Debug \
		-DGODOT_PRECISION=double \
		-DGODOT_USE_HOT_RELOAD=ON \
		-DCMAKE_INSTALL_PREFIX=$(SOURCE_DIR)/demo

# Build the project
build: $(BUILD_DIR)/$(OUTPUT_ARTIFACT)
$(BUILD_DIR)/$(OUTPUT_ARTIFACT): $(BUILD_DIR)/Makefile $(SRC_FILES)
	cmake --build $(BUILD_DIR) --parallel 8

# Install the project
install: $(SOURCE_DIR)/demo/bin/$(OUTPUT_ARTIFACT)
$(SOURCE_DIR)/demo/bin/$(OUTPUT_ARTIFACT): $(BUILD_DIR)/$(OUTPUT_ARTIFACT)
	cmake --install $(BUILD_DIR)

install: build
	cp $(BUILD_DIR)/libPureDataGD.so ./demo/bin/

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(SOURCE_DIR)/demo/bin/$(OUTPUT_ARTIFACT)

# Remove compile_commands.json
clean-commands:
	rm -f $(BUILD_DIR)/compile_commands.json

# Phony targets to avoid conflict with file names
.PHONY: all configure build clean clean-commands
