#pragma once
#include <vulkan/vulkan.h>
#include "VulkanTypes.hpp"
class PipelineStrategy{
    public:
    virtual ~PipelineStrategy() = default;
    virtual VkPipelinePtr createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout) = 0;
};
