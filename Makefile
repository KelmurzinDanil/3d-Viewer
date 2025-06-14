BUILD_DIR := build
APP_EXECUTABLE := $(BUILD_DIR)/Debug/VulkanApp.exe
TEST_EXECUTABLE := $(BUILD_DIR)/tests/Debug/VulkanTests.exe

all: run

$(BUILD_DIR)/Makefile:
	mkdir -p "$(BUILD_DIR)"
	cd "$(BUILD_DIR)" && cmake .. -G "Visual Studio 17 2022"

$(APP_EXECUTABLE): $(BUILD_DIR)/Makefile
	cd "$(BUILD_DIR)" && cmake --build . --config Debug

run: $(APP_EXECUTABLE)
	@echo "Запуск приложения..."
	@cd "$(BUILD_DIR)/Debug" && ./VulkanApp.exe

# Новая цель: запуск тестов
test: $(TEST_EXECUTABLE)
	@echo "Запуск тестов..."
	@cd "$(BUILD_DIR)\tests\Debug" && VulkanTests.exe

$(TEST_EXECUTABLE): $(BUILD_DIR)/Makefile
	cd "$(BUILD_DIR)" && cmake --build . --config Debug --target VulkanTests

clean:
	@rm -rf "$(BUILD_DIR)" || rmdir /s /q "$(BUILD_DIR)" 2>nul || exit 0

.PHONY: all run test clean