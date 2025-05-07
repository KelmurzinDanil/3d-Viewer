#include "BufferManager.hpp"

BufferManager::BufferManager(DeviceManager& deviceManager,
                             VkCommandPool commandPool,
                             SwapChainManager& swapChainManager) 
: deviceManager_(deviceManager),
commandPool_(commandPool),
swapChainManager_(swapChainManager),
indexBuffer(nullptr, VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(nullptr)),
indexBufferMemory(nullptr, VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(nullptr)),
vertexBuffer(nullptr, VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(nullptr)),
vertexBufferMemory(nullptr, VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(nullptr))
{
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
}

const std::vector<VkBufferPtr>& BufferManager::getUniformBuffers() const {
    return uniformBuffers; }
const std::vector<void*>& BufferManager::getUniformBuffersMapped() const {
     return uniformBuffersMapped; }

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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

void BufferManager::createIndexBuffer() {
    if (Constants::indices.empty()) {
        throw std::runtime_error("Index data is empty!");
    }

    VkDeviceSize bufferSize = sizeof(Constants::indices[0]) * Constants::indices.size();

    // Создание staging буфера
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
    memcpy(data, Constants::indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(deviceManager_.device(), stagingBufferMemory);

    // Создание основного индексного буфера
    VkBuffer rawIndexBuffer;
    VkDeviceMemory rawIndexBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        rawIndexBuffer,
        rawIndexBufferMemory
    );

    // Копирование данных из staging в основной буфер
    copyBuffer(stagingBuffer, rawIndexBuffer, bufferSize);

    // Оборачивание в умные указатели
    indexBuffer = VkBufferPtr(
        rawIndexBuffer,
        VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(deviceManager_.device())
    );
    indexBufferMemory = VkDeviceMemoryPtr(
        rawIndexBufferMemory,
        VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(deviceManager_.device())
    );

    // Очистка staging ресурсов
    vkDestroyBuffer(deviceManager_.device(), stagingBuffer, nullptr);
    vkFreeMemory(deviceManager_.device(), stagingBufferMemory, nullptr);
}

void BufferManager::createVertexBuffer() {
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
void BufferManager::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
        VkBuffer buffer;
        VkDeviceMemory memory;

        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            buffer, memory);

        uniformBuffers.emplace_back(
            buffer,
            VulkanDeleter<VkBuffer_T, vkDestroyBuffer, VkDevice>(deviceManager_.device()));
    
        uniformBuffersMemory.emplace_back(
            memory,
            VulkanDeleter<VkDeviceMemory_T, vkFreeMemory, VkDevice>(deviceManager_.device()));
        
        vkMapMemory(deviceManager_.device(), uniformBuffersMemory.back().get(), 
            0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool_;
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

    vkFreeCommandBuffers(deviceManager_.device(), commandPool_, 1, &commandBuffer);
}