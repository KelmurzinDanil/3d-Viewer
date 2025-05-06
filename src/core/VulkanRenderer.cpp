#include "VulkanRenderer.hpp"

VulkanRenderer::VulkanRenderer(WindowManager& windowManager, 
                               DeviceManager& deviceManager,
                               SwapChainManager& swapChainManager,
                               PipelineManager& pipelineManager,
                               InstanceManager& instanceManager,
                               SurfaceManager& surfaceManager,
                               CommandManager& commandManager,
                               BufferManager& bufferManager,
                               bool enableValidationLayers)
    : windowManager_(windowManager),
      deviceManager_(deviceManager),
      swapChainManager_(swapChainManager),
      pipelineManager_(pipelineManager),
      instanceManager_(instanceManager),
      surfaceManager_(surfaceManager),
      commandManager_(commandManager),
      bufferManager_(bufferManager),
      enableValidationLayers_(enableValidationLayers),
      renderPass(nullptr, VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>(nullptr)),
      graphicsPipeline(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr))
       {
        pipelineManager_.createPipelineLayout();
        pipelineManager_.createGraphicsPipeline();
      }

void VulkanRenderer::drawFrame() {
    // Получаем сырые указатели из умных
    VkFence rawInFlightFence = commandManager_.inFlightFence();
    VkSemaphore rawImageAvailableSemaphore = commandManager_.imageAvailableSemaphore();
    VkSemaphore rawRenderFinishedSemaphore = commandManager_.renderFinishedSemaphore();

    if (rawInFlightFence == VK_NULL_HANDLE ||  //Потом удалить!!
        rawImageAvailableSemaphore == VK_NULL_HANDLE ||
        rawRenderFinishedSemaphore == VK_NULL_HANDLE) {
        throw std::runtime_error("Invalid synchronization objects!");
    } 

    // Ожидаем завершение предыдущего кадра
    vkWaitForFences(deviceManager_.device(), 1, &rawInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(deviceManager_.device(), 1, &rawInFlightFence);


    // Получаем индекс изображения из цепочки подкачки
    uint32_t imageIndex;

    //vkAcquireNextImageKHR(
    //    deviceManager_.device(), 
    //    swapChainManager_.swapChain.get(), 
    //    UINT64_MAX, 
    //    rawImageAvailableSemaphore, 
    //    VK_NULL_HANDLE, 
    //    &imageIndex
    //);

    VkResult result = vkAcquireNextImageKHR(deviceManager_.device(), //Потом удалить!!
    swapChainManager_.swapChain.get(), 
    UINT64_MAX, 
    rawImageAvailableSemaphore, 
    VK_NULL_HANDLE, 
    &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { //Потом удалить!!
        throw std::runtime_error("Failed to acquire image!");
    }
    VkCommandBuffer commandBuffer = commandManager_.getCommandBuffer(); //Потом удалить!!
    if (commandBuffer == VK_NULL_HANDLE) {                              //Потом удалить!!
        throw std::runtime_error("Command buffer is not initialized!");
    }

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

    VkSwapchainKHR swapChains[] = {swapChainManager_.swapChain.get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    // Отображаем кадр
    vkQueuePresentKHR(swapChainManager_.getPresentQueue(), &presentInfo);
}
