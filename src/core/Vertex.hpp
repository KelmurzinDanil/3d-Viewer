#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>



struct Vertex{
    glm::vec2 pos;
    glm::vec3 color;
    
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        /**Параметр binding указывает индекс привязки в массиве привязок.
         * Параметр stride указывает количество байтов от одной записи до следующей, 
         * а параметр inputRate может принимать одно из следующих значений:
         * VK_VERTEX_INPUT_RATE_VERTEX: Переход к следующей записи данных после каждой вершины
         * VK_VERTEX_INPUT_RATE_INSTANCE: Переход к следующей записи данных после каждого экземпляра 
         **/
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    /***/
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        /**Параметр binding сообщает Vulkan, из какого связывания поступают данные для каждой вершины.
         * Параметр location ссылается на директиву location ввода в вершинном шейдере.
         * Параметр format описывает тип данных для атрибута.
         * Параметр format неявно определяет размер в байтах данных атрибутов,
         *  а параметр offset указывает количество байтов, начиная с которых следует считывать данные для каждой вершины.*/

        // Position (Location 0)
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        
        // Color (Location 1)
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

extern const std::vector<Vertex> vertices; 

