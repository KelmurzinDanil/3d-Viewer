#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "DeviceManager.hpp"
#include "VulkanTypes.hpp"
#include "BufferManager.hpp"
#include "SwapChainManager.hpp"
#include "PipelineManager.hpp"

class CommandManager {
public:
    CommandManager(DeviceManager& deviceManager, SwapChainManager& swapChainManager, PipelineManager& pipelineManager);

    DeviceManager& deviceManager_;
    SwapChainManager& swapChainManager_;

    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer_, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer);
    
    
    VkCommandPool commandPool() const { return commandPool_.get(); }

    VkSemaphorePtr imageAvailableSemaphore_;
    VkSemaphorePtr renderFinishedSemaphore_;
    VkFencePtr inFlightFence_;

    const VkCommandBuffer getCommandBuffer() const {return commandBuffer_;}
    const VkCommandBuffer* getCommandBuffer2() const {
        return &commandBuffer_;
    }

    VkSemaphore imageAvailableSemaphore() const { return imageAvailableSemaphore_.get(); }
    VkSemaphore renderFinishedSemaphore() const { return renderFinishedSemaphore_.get(); }
    VkFence inFlightFence() const { return inFlightFence_.get(); }
    VkCommandPool getCommandPool() const {return commandPool_.get();}

    
private:
    VkCommandPoolPtr commandPool_;
    VkCommandBuffer commandBuffer_;

    PipelineManager& pipelineManager_;

    size_t currentFrame_ = 0;

};