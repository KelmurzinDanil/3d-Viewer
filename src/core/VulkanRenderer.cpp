#include "VulkanRenderer.hpp"

VulkanRenderer::VulkanRenderer(WindowManager& windowManager, 
                               DeviceManager& deviceManager,
                               SwapChainManager& swapChainManager,
                               PipelineManager& pipelineManager,
                               InstanceManager& instanceManager,
                               SurfaceManager& surfaceManager,
                               CommandManager& commandManager,
                               bool enableValidationLayers)
    : windowManager_(windowManager),
      deviceManager_(deviceManager),
      swapChainManager_(swapChainManager),
      pipelineManager_(pipelineManager),
      instanceManager_(instanceManager),
      surfaceManager_(surfaceManager),
      commandManager_(commandManager),
      enableValidationLayers_(enableValidationLayers),
      renderPass(nullptr, VulkanDeleter<VkRenderPass_T, vkDestroyRenderPass, VkDevice>(nullptr)),
      graphicsPipeline(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr)),
      vertexBuffer(nullptr, VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(nullptr)),
      vertexBufferMemory(nullptr, VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(nullptr))
       {
        checkBindingsConsistency();
        validateVertexAttributes();
        pipelineManager_.createPipelineLayout();
        pipelineManager_.createGraphicsPipeline();
        createVertexBuffer();
      }


void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // Используем переданный размер
    bufferInfo.usage = usage; // Используем переданные флаги
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(deviceManager_.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceManager_.device(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanUtils::findMemoryType(
        deviceManager_.physicalDevice(),
        memRequirements.memoryTypeBits,
        properties // Используем переданные свойства памяти
    );

    if (vkAllocateMemory(deviceManager_.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(deviceManager_.device(), buffer, bufferMemory, 0);
}

void VulkanRenderer::createVertexBuffer() {
    if (vertices.empty()) {
        throw std::runtime_error("Vertex data is empty!");
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Создание staging ресурсов
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    // Заполнение staging буфера
    void* data;
    vkMapMemory(deviceManager_.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(deviceManager_.device(), stagingBufferMemory);

    // Создание основного буфера
    VkBuffer rawVertexBuffer;
    VkDeviceMemory rawVertexBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        rawVertexBuffer,
        rawVertexBufferMemory
    );

    // Копирование данных
    copyBuffer(stagingBuffer, rawVertexBuffer, bufferSize);

    // Оборачиваем в умные указатели
    vertexBuffer = VkBufferPtr(
        rawVertexBuffer,
        VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(deviceManager_.device())
    );
    vertexBufferMemory = VkDeviceMemoryPtr(
        rawVertexBufferMemory,
        VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(deviceManager_.device())
    );

    // Очистка staging ресурсов
    vkDestroyBuffer(deviceManager_.device(), stagingBuffer, nullptr);
    vkFreeMemory(deviceManager_.device(), stagingBufferMemory, nullptr);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandManager_.getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(deviceManager_.device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        /**Содержимое буферов передаётся с помощью команды vkCmdCopyBuffer.
         *  Она принимает в качестве аргументов исходный и целевой буферы,
         *  а также массив областей для копирования.  */
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(swapChainManager_.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(swapChainManager_.getGraphicsQueue());

    vkFreeCommandBuffers(deviceManager_.device(), commandManager_.getCommandPool(), 1, &commandBuffer);
}


void VulkanRenderer::validateVertexAttributes() {
    auto attributes = Vertex::getAttributeDescriptions();
    
    // Получаем свойства физического устройства (включая лимиты)
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(deviceManager_.physicalDevice(), &deviceProps);

    for (const auto& attr : attributes) {
        // 1. Проверка поддержки формата для вершинных буферов
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(deviceManager_.physicalDevice(), attr.format, &formatProps);
        if (!(formatProps.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
            throw std::runtime_error("Format " + std::to_string(attr.format) + " not supported for vertex buffers!");
        }

        // 2. Проверка binding (должен быть < maxVertexInputBindings)
        if (attr.binding >= deviceProps.limits.maxVertexInputBindings) {
            throw std::runtime_error("Binding " + std::to_string(attr.binding) 
                + " exceeds device limit (" 
                + std::to_string(deviceProps.limits.maxVertexInputBindings) 
                + ")!");
        }

        // 3. Проверка location (должен быть < maxVertexInputAttributes)
        if (attr.location >= deviceProps.limits.maxVertexInputAttributes) {
            throw std::runtime_error("Location " + std::to_string(attr.location) 
                + " exceeds device limit (" 
                + std::to_string(deviceProps.limits.maxVertexInputAttributes) 
                + ")!");
        }
    }
}

void VulkanRenderer::checkBindingsConsistency() {
    auto bindings = { Vertex::getBindingDescription() };
    auto attributes = Vertex::getAttributeDescriptions();

    for (const auto& attr : attributes) {
        bool found = false;
        for (const auto& binding : bindings) {
            if (attr.binding == binding.binding) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Attribute references non-existent binding!");
        }
    }
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
    commandManager_.recordCommandBuffer(commandManager_.getCommandBuffer(), imageIndex, vertexBuffer.get());

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
