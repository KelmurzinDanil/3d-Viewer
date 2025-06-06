#pragma once
#include <vulkan/vulkan.h>
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

    
    inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        //Структура VkPhysicalDeviceMemoryProperties содержит два массива memoryTypes и memoryHeaps
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
 
        //Найдём тип памяти, подходящий для самого буфера:
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            //(1)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)  {
                return i;
            }
        }
        
        throw std::runtime_error("failed to find suitable memory type!");
    }

    inline VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
         VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice) {
        for (VkFormat format : candidates) {
            // linearTilingFeatures - Варианты использования, поддерживаемые линейной мозаикой
            // ptimalTilingFeatures - Варианты использования, поддерживаемые оптимальной мозаичной раскладкой 
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }
    inline VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, // Кандидаты 
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
            physicalDevice
        );
    }
    inline bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
}

/** 1. Параметр typeFilter будет использоваться для указания битового поля подходящих типов памяти.
 *  Это означает, что мы можем найти индекс подходящего типа памяти, просто перебирая их и проверяя,
 *  установлен ли соответствующий бит в 1.
 *  Однако нас интересует не только тип памяти, подходящий для вершинного буфера.
 *  Нам также нужно иметь возможность записывать данные вершин в эту память.
 *  Массив memoryTypes состоит из VkMemoryType структур, которые определяют кучу и свойства каждого типа памяти.
 *  Свойства определяют особые характеристики памяти, например возможность сопоставления с ней, чтобы мы могли записывать в неё из процессора.
 *  Это свойство обозначается VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, но нам также нужно использовать свойство VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.*/