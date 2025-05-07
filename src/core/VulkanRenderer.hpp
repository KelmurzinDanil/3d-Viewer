#pragma once
//#define GLM_FORCE_RADIANS
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "CommandManager.hpp"
#include "PipelineBuilder.hpp"
#include "PipelineStrategy.hpp"
#include "BasicTriangleStrategy.hpp"
#include "PipelineManager.hpp"
#include "Vertex.hpp"
#include "BufferManager.hpp"
#include <memory>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>




class VulkanRenderer {
public:

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;


    VulkanRenderer(WindowManager& windowManager, DeviceManager& deviceManager,
         SwapChainManager& swapChainManager, PipelineManager& pipelineManager, InstanceManager& instanceManager, SurfaceManager& surfaceManager,
         CommandManager& commandManager, BufferManager& bufferManager,
          bool enableValidationLayers);

    /**
     * @brief Отрисовывает один кадр
     * Управляет синхронизацией и отправкой команд в GPU
     */
    void drawFrame();

    void updateUniformBuffer(uint32_t currentImage);

private:
    
    // Ссылки на менеджеры (владение объектами остается за ними)
    InstanceManager& instanceManager_;
    DeviceManager& deviceManager_;
    SurfaceManager& surfaceManager_;
    SwapChainManager& swapChainManager_;
    PipelineManager& pipelineManager_;
    CommandManager& commandManager_;
    WindowManager& windowManager_;
    BufferManager& bufferManager_;
  
    bool enableValidationLayers_;///< Флаг использования слоев валидации

    uint32_t currentFrame = 0;

    VkRenderPassPtr renderPass;
    VkPipelinePtr graphicsPipeline;
    std::vector<VkFramebufferPtr> swapChainFramebuffers;
    

    
    std::unique_ptr<PipelineStrategy> pipelineStrategy;
};