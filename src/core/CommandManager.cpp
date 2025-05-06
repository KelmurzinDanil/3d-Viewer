#include "CommandManager.hpp"


CommandManager::CommandManager(DeviceManager& deviceManager, 
    SwapChainManager& swapChainManager, PipelineManager& pipelineManager) : deviceManager_(deviceManager),
    swapChainManager_(swapChainManager),pipelineManager_(pipelineManager),
    commandPool_(nullptr, VulkanDeleter<VkCommandPool_T, vkDestroyCommandPool, VkDevice>(nullptr)),
    imageAvailableSemaphore_(nullptr, VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(nullptr)),
    renderFinishedSemaphore_(nullptr, VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(nullptr)),
    inFlightFence_(nullptr, VulkanDeleter<VkFence_T, vkDestroyFence, VkDevice>(nullptr))
{
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}




void CommandManager::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = deviceManager_.findQueueFamilies(deviceManager_.physicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkCommandPool rawCommandPool;
    if (vkCreateCommandPool(deviceManager_.device(), &poolInfo, nullptr, &rawCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    commandPool_ = VkCommandPoolPtr(rawCommandPool, 
        VulkanDeleter<VkCommandPool_T, vkDestroyCommandPool, VkDevice>(deviceManager_.device()));
}

void CommandManager::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_.get();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(deviceManager_.device(), &allocInfo, &commandBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void CommandManager::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChainManager_.renderPass.get();
    renderPassInfo.framebuffer = swapChainManager_.swapChainFramebuffers[imageIndex].get();
    renderPassInfo.renderArea.extent = swapChainManager_.getExtent();
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager_.getGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainManager_.getSwapChainExtent().width;
    viewport.height = (float) swapChainManager_.getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainManager_.getSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor); 

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(BufferManager::indices.size()), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphore rawImageAvailableSemaphore;
    VkSemaphore rawRenderFinishedSemaphore;
    VkFence rawInFlightFence;

    if (vkCreateSemaphore(deviceManager_.device(), &semaphoreInfo, nullptr, &rawImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(deviceManager_.device(), &semaphoreInfo, nullptr, &rawRenderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(deviceManager_.device(), &fenceInfo, nullptr, &rawInFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects!");
    }

    imageAvailableSemaphore_ = VkSemaphorePtr(
        rawImageAvailableSemaphore,
        VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(
            deviceManager_.device()
        )
    );

    renderFinishedSemaphore_ = VkSemaphorePtr(
        rawRenderFinishedSemaphore,
        VulkanDeleter<VkSemaphore_T, vkDestroySemaphore, VkDevice>(
            deviceManager_.device()
        )
    );

    inFlightFence_ = VkFencePtr(
        rawInFlightFence,
        VulkanDeleter<VkFence_T, vkDestroyFence, VkDevice>(
            deviceManager_.device()
        )
    );
}
