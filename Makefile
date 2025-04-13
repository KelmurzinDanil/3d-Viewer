BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/Debug/VulkanApp.exe

all: run

$(BUILD_DIR)/Makefile:
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	cd $(BUILD_DIR) && cmake .. -G "Visual Studio 17 2022"

$(EXECUTABLE): $(BUILD_DIR)/Makefile
	cd $(BUILD_DIR) && cmake --build . --config Debug

run: $(EXECUTABLE)
	@echo "Запуск приложения..."
	@cd $(BUILD_DIR)/Debug && .\VulkanApp.exe

clean:
	@rmdir /s /q $(BUILD_DIR) 2>nul || exit 0

.PHONY: all run clean