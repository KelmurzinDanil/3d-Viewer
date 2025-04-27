#pragma once
//#define NOMINMAX
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "CommandManager.hpp"
#include "PipelineBuilder.hpp"
#include "PipelineStrategy.hpp"
#include "BasicTriangleStrategy.hpp"
#include "PipelineManager.hpp"
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
    // Ссылки на менеджеры (владение объектами остается за ними)
    InstanceManager& instanceManager_;
    DeviceManager& deviceManager_;
    SurfaceManager& surfaceManager_;
    SwapChainManager& swapChainManager_;
    PipelineManager& pipelineManager_;
    CommandManager& commandManager_;
    WindowManager& windowManager_;

  
    bool enableValidationLayers_;///< Флаг использования слоев валидации
    
    VkRenderPassPtr renderPass;
    VkPipelinePtr graphicsPipeline;
    std::vector<VkFramebufferPtr> swapChainFramebuffers;
    

    
    std::unique_ptr<PipelineStrategy> pipelineStrategy;
};