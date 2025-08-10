BUILD_DIR := build
SRC_FILES := $(shell find src -name '*.cpp')
BUILD_TYPE ?= Release
CPP_FILES := $(shell find src -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \))

.PHONY: all configure build clean rebuild lint format format-check

all: build

configure:
	cmake -B $(BUILD_DIR)/build/$(BUILD_TYPE) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	ln -sf $(BUILD_DIR)/build/$(BUILD_TYPE)/compile_commands.json compile_commands.json

build: configure
	cmake --build $(BUILD_DIR)/build/$(BUILD_TYPE)

run: build
	./build/build/Release/fiber

debug-configure:
	$(MAKE) BUILD_TYPE=Debug configure

debug:
	$(MAKE) BUILD_TYPE=Debug build

debug-run: debug
	./build/build/Debug/fiber

clean:
	rm -rf $(BUILD_DIR)
	rm ./compile_commands.json

rebuild: clean all

lint: configure
	clang-tidy $(CPP_FILES) --extra-arg=-Isrc --extra-arg=-std=c++23 -p $(BUILD_DIR)/build/$(BUILD_TYPE)

format: configure
	clang-format -i $(CPP_FILES)

format-check: configure
	clang-format --dry-run -Werror $(CPP_FILES)

