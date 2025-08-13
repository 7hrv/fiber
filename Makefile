BUILD_DIR := build
SRC_FILES := $(shell find src -name '*.cpp')
BUILD_TYPE ?= Release
CPP_FILES := $(shell find src -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \))

.PHONY: all configure build clean rebuild lint format format-check

all: build

configure:
	mkdir -p $(BUILD_DIR)
	conan install . --output-folder=$(BUILD_DIR) --build=missing --settings=build_type=$(BUILD_TYPE)
	cmake --preset conan-$(shell echo $(BUILD_TYPE) | tr A-Z a-z)
	ln -sf $(BUILD_DIR)/build/$(BUILD_TYPE)/compile_commands.json compile_commands.json

build: configure
	cmake --build $(BUILD_DIR)/build/$(BUILD_TYPE)

run: build
	./$(BUILD_DIR)/build/$(BUILD_TYPE)/fiber

run-benchmark: build
	./$(BUILD_DIR)/build/$(BUILD_TYPE)/benchmarks/bench_handoff

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

