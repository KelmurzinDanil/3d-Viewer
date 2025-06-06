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
        swapChainManager_.getRenderPass(),
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


    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    // собрать информацию о дескрипторном наборе в одну структуру.
    VkDescriptorSetLayoutCreateInfo layoutInfo{};   
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout rawDescriptorSetLayout;
    if (vkCreateDescriptorSetLayout(deviceManager_.device(), &layoutInfo, nullptr, &rawDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    descriptorSetLayout = VkDescriptorSetLayoutPtr(rawDescriptorSetLayout,
         VulkanDeleter<VkDescriptorSetLayout_T, vkDestroyDescriptorSetLayout, VkDevice>(deviceManager_.device()));
}
void PipelineManager::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPool rawDescriptorPool;
    if (vkCreateDescriptorPool(deviceManager_.device(), &poolInfo, nullptr, &rawDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    descriptorPool = VkDescriptorPoolPtr(rawDescriptorPool,
         VulkanDeleter<VkDescriptorPool_T, vkDestroyDescriptorPool, VkDevice>(deviceManager_.device()));
    
}

void PipelineManager::createDescriptorSets(const std::vector<VkBufferPtr>& uniformBuffers, VkSampler textureSampler, VkImageView textureImageView) {
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

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets( deviceManager_.device(),
                                    static_cast<uint32_t>(descriptorWrites.size()),
                                    descriptorWrites.data(),
                                    0,
                                    nullptr);
        }
}