#include "PipelineManager.hpp"
PipelineManager::PipelineManager(DeviceManager& deviceMgr, SwapChainManager& swapMgr)
    : deviceManager_(deviceMgr), swapChainManager_(swapMgr),
    graphicsPipeline_(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr)),
    pipelineLayout_(nullptr, VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>(nullptr))
    {}

void PipelineManager::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; 
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkPipelineLayout rawLayout;
    if (vkCreatePipelineLayout(
        deviceManager_.device(), 
        &pipelineLayoutInfo, 
        nullptr, 
        &rawLayout
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    pipelineLayout_ = VkPipelineLayoutPtr(
        rawLayout, 
        VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>(
            deviceManager_.device()
        )
    );
}

void PipelineManager::createGraphicsPipeline() {
    BasicTriangleStrategy strategy;
    graphicsPipeline_ = strategy.createGraphicsPipeline(
        deviceManager_.device(),
        swapChainManager_.renderPass.get(),
        pipelineLayout_.get()
    );
}