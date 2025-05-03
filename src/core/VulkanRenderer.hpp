#pragma once
//#define NOMINMAX
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "CommandManager.hpp"
#include "PipelineBuilder.hpp"
#include "PipelineStrategy.hpp"
#include "BasicTriangleStrategy.hpp"
#include "PipelineManager.hpp"
#include "Vertex.hpp"
//#include "VulkanUtils.hpp"
#include <memory>

class VulkanRenderer {
public:

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;


    VulkanRenderer(WindowManager& windowManager, DeviceManager& deviceManager,
         SwapChainManager& swapChainManager, PipelineManager& pipelineManager, InstanceManager& instanceManager, SurfaceManager& surfaceManager,
         CommandManager& commandManager,
          bool enableValidationLayers);

    /**
     * @brief Отрисовывает один кадр
     * Управляет синхронизацией и отправкой команд в GPU
     */
    void drawFrame();
    

private:
    /**
     * @brief Создает вершинный буфер и выделяет память
     */
    void createVertexBuffer();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void checkBindingsConsistency();
    void validateVertexAttributes();
    // Ссылки на менеджеры (владение объектами остается за ними)
    InstanceManager& instanceManager_;
    DeviceManager& deviceManager_;
    SurfaceManager& surfaceManager_;
    SwapChainManager& swapChainManager_;
    PipelineManager& pipelineManager_;
    CommandManager& commandManager_;
    WindowManager& windowManager_;

  
    bool enableValidationLayers_;///< Флаг использования слоев валидации
    VkBuffer rawVertexBuffer;
    VkDeviceMemory rawVertexBufferMemory;

    VkDeviceMemoryPtr vertexBufferMemory;
    VkBufferPtr vertexBuffer;  ///< Вершинный буфер
    VkRenderPassPtr renderPass;
    VkPipelinePtr graphicsPipeline;
    std::vector<VkFramebufferPtr> swapChainFramebuffers;
    

    
    std::unique_ptr<PipelineStrategy> pipelineStrategy;
};