#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>
#include "VulkanRenderer.hpp"

class PipelineBuilder {
public:
    PipelineBuilder(VkDevice device, VkRenderPass renderPass);
    
    // Устанавливает вершинный и фрагментный шейдеры
    PipelineBuilder& setShaders(const std::string& vertPath, const std::string& fragPath);
    
    // Конфигурирует входную сборку (топология примитивов)
    PipelineBuilder& setInputAssembly(VkPrimitiveTopology topology);
    
    // Устанавливает viewport и область отсечения (scissor)
    PipelineBuilder& setViewport(VkExtent2D extent);
    
    // Задает режим отсечения граней (например, VK_CULL_MODE_NONE)
    PipelineBuilder& setCullMode(VkCullModeFlags cullMode);
    
    // Устанавливает режим отрисовки полигонов (заполнение, каркас и т.д.)
    PipelineBuilder& setPolygonMode(VkPolygonMode mode);
    
    // Настраивает динамические состояния (например, viewport/scissor)
    PipelineBuilder& setDynamicState(std::vector<VkDynamicState> dynamicStates);
    
    // Конфигурирует смешивание цветов (наследует логику из примера)
    PipelineBuilder& setColorBlending();
    
    PipelineBuilder& setRasterizer();

    PipelineBuilder& enableDepthTest(bool enable);

    PipelineBuilder& enableBlending();

    PipelineBuilder& setMultisampling();

    PipelineBuilder& setPipelineLayout(VkPipelineLayout layout);

    // Создает и возвращает готовый конвейер
    VkPipelinePtr build();

private:
    VkDevice device; 
    VkRenderPass renderPass;
    VkViewport viewport{};  
    VkRect2D scissor{};  
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages; // Шейдерные стадии
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; // Входные данные вершин
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; // Входная сборка
    VkPipelineViewportStateCreateInfo viewportState{}; // Состояние viewport/scissor
    VkPipelineRasterizationStateCreateInfo rasterizer{}; // Растеризатор
    VkPipelineMultisampleStateCreateInfo multisampling{}; // Мультисэмплинг
    VkPipelineColorBlendStateCreateInfo colorBlending{}; // Смешивание цветов
    VkPipelineDynamicStateCreateInfo dynamicState{}; // Динамические состояния
    VkPipelineLayout pipelineLayout; // Макет конвейера
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // Состояние смешивания для вложений
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    std::vector<VkDynamicState> dynamicStates_;
};