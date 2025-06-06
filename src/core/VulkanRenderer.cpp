#include "VulkanRenderer.hpp"

VulkanRenderer::VulkanRenderer(WindowManager& windowManager, 
                               DeviceManager& deviceManager,
                               SwapChainManager& swapChainManager,
                               PipelineManager& pipelineManager,
                               InstanceManager& instanceManager,
                               SurfaceManager& surfaceManager,
                               CommandManager& commandManager,
                               BufferManager& bufferManager,
                               TextureManager& textureManager,
                               bool enableValidationLayers)
    : windowManager_(windowManager),
      deviceManager_(deviceManager),
      swapChainManager_(swapChainManager),
      pipelineManager_(pipelineManager),
      instanceManager_(instanceManager),
      surfaceManager_(surfaceManager),
      commandManager_(commandManager),
      bufferManager_(bufferManager),
      textureManager_(textureManager),
      enableValidationLayers_(enableValidationLayers),
      renderPass(nullptr, VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>(nullptr)),
      graphicsPipeline(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr))

       {
        pipelineManager_.createDescriptorSetLayout();
        pipelineManager_.createPipelineLayout();
        pipelineManager_.createGraphicsPipeline();
        pipelineManager_.createDescriptorPool();
        pipelineManager_.createDescriptorSets(bufferManager_.getUniformBuffers(),
                                                textureManager_.getTextureSampler(),
                                                textureManager_.getTextureImageView());
      }

void VulkanRenderer::drawFrame() {
    // Получаем сырые указатели из умных
    VkFence rawInFlightFence = commandManager_.inFlightFence();
    VkSemaphore rawImageAvailableSemaphore = commandManager_.imageAvailableSemaphore();
    VkSemaphore rawRenderFinishedSemaphore = commandManager_.renderFinishedSemaphore();

    // Ожидаем завершение предыдущего кадра
    vkWaitForFences(deviceManager_.device(), 1, &rawInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(deviceManager_.device(), 1, &rawInFlightFence);


    // Получаем индекс изображения из цепочки подкачки
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(deviceManager_.device(), 
    swapChainManager_.getSwapChain(), 
    UINT64_MAX, 
    rawImageAvailableSemaphore, 
    VK_NULL_HANDLE, 
    &imageIndex);


    updateUniformBuffer(currentFrame);


    VkCommandBuffer commandBuffer = commandManager_.getCommandBuffer();
    
    // Подготавливаем командный буфер
    vkResetCommandBuffer(commandManager_.getCommandBuffer(), 0);
    commandManager_.recordCommandBuffer(commandManager_.getCommandBuffer(), imageIndex,
                             bufferManager_.getVertexBuffer(), bufferManager_.getIndexBuffer());


    // Настраиваем информацию для отправки команд
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Семафоры для синхронизации этапов рендеринга
    VkSemaphore waitSemaphores[] = {rawImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandManager_.getCommandBuffer2();

    VkSemaphore signalSemaphores[] = {rawRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Отправляем командный буфер в очередь
    if (vkQueueSubmit(swapChainManager_.getGraphicsQueue(), 1, &submitInfo, rawInFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Настраиваем информацию для отображения кадра
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChainManager_.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    // Отображаем кадр
    vkQueuePresentKHR(swapChainManager_.getPresentQueue(), &presentInfo);
}
void VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainManager_.getSwapChainExtent().width / (float) swapChainManager_.getSwapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(bufferManager_.getUniformBuffersMapped()[currentImage], &ubo, sizeof(ubo));
}