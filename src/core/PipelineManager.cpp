#include "PipelineManager.hpp"


PipelineManager::PipelineManager(DeviceManager& deviceMgr, SwapChainManager& swapMgr)
    : deviceManager_(deviceMgr), swapChainManager_(swapMgr), 
    graphicsPipeline_(nullptr, VulkanDeleter<VkPipeline_T, vkDestroyPipeline, VkDevice>(nullptr)),
    pipelineLayout_(nullptr, VulkanDeleter<VkPipelineLayout_T, vkDestroyPipelineLayout, VkDevice>(nullptr)),
    descriptorSetLayout(nullptr, VulkanDeleter<VkDescriptorSetLayout_T, vkDestroyDescriptorSetLayout, VkDevice>(nullptr)),
    descriptorPool(nullptr, VulkanDeleter<VkDescriptorPool_T, vkDestroyDescriptorPool, VkDevice>(nullptr))
    {}

void PipelineManager::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; 
    VkDescriptorSetLayout rawDescriptorLayout = descriptorSetLayout.get();

    pipelineLayoutInfo.pSetLayouts = &rawDescriptorLayout;
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
void PipelineManager::createDescriptorSetLayout() {

    // сообщить Vulkan, что в вершинном шейдере будет использоваться один uniform-буфер, привязанный к индексу 0
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // собрать информацию о дескрипторном наборе в одну структуру.
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout rawDescriptorSetLayout;
    if (vkCreateDescriptorSetLayout(deviceManager_.device(), &layoutInfo, nullptr, &rawDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    descriptorSetLayout = VkDescriptorSetLayoutPtr(rawDescriptorSetLayout,
         VulkanDeleter<VkDescriptorSetLayout_T, vkDestroyDescriptorSetLayout, VkDevice>(deviceManager_.device()));
}
void PipelineManager::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPool rawDescriptorPool;
    if (vkCreateDescriptorPool(deviceManager_.device(), &poolInfo, nullptr, &rawDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    descriptorPool = VkDescriptorPoolPtr(rawDescriptorPool,
         VulkanDeleter<VkDescriptorPool_T, vkDestroyDescriptorPool, VkDevice>(deviceManager_.device()));
    
}

void PipelineManager::createDescriptorSets(const std::vector<VkBufferPtr>& uniformBuffers) {
    std::vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout.get());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool.get();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(deviceManager_.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].get();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(deviceManager_.device(), 1, &descriptorWrite, 0, nullptr);
        }
}