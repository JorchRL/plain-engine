BUILD ?= build/debug
TYPE ?= Debug

.PHONY: all run clean
all:
	cmake -S . -B $(BUILD) -G Ninja -DCMAKE_BUILD_TYPE=$(TYPE)
	cmake --build $(BUILD) -j
	ln -sf build/debug/compile_commands.json .
run: all
	./$(BUILD)/bin/pacman
clean:
	rm -rf build
