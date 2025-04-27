#pragma once
#include <vector>
#include <string>
#include <fstream>

namespace VulkanUtils {
    /**
     * @brief Читает бинарный файл в вектор байтов
     * 
     * @param filename Путь к файлу
     * @return std::vector<char> Содержимое файла
     * 
     * Особенности реализации:
     * 1. Использует std::ios::ate для определения размера файла
     * 2. Считывает файл с конца (эффективно для больших файлов)
     * 3. Гарантирует закрытие файла при выходе из функции
     * 4. Выбрасывает исключение при ошибке доступа к файлу
     */
    inline std::vector<char> readFile(const std::string& filename) {
        // Открываем файл в бинарном режиме с позиционированием в конец
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        // Определяем размер файла по текущей позиции (в конце)
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        // Возвращаемся в начало файла для чтения
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    /**
     * @brief Создает Vulkan-модуль шейдера
     * 
     * @param device Логическое устройство Vulkan
     * @param code Бинарный код шейдера (SPIR-V)
     * @return VkShaderModule Созданный модуль шейдера
     * 
     * Требования:
     * 1. Код должен быть в формате SPIR-V
     * 2. Размер кода должен быть кратен 4 (требование Vulkan)
     */
    inline VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;
    }

}

