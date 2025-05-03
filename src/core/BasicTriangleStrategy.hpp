#pragma once
#include "PipelineStrategy.hpp"
#include <vector>
#include "PipelineBuilder.hpp"

class BasicTriangleStrategy : public PipelineStrategy {
public:
    VkPipelinePtr createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout) override {
        PipelineBuilder builder(device, renderPass);
        std::vector<VkDynamicState> dynamicStates{
            VK_DYNAMIC_STATE_VIEWPORT, 
            VK_DYNAMIC_STATE_SCISSOR
        };
        return builder
            .setShaders(SHADER_DIR "/vert.spv", SHADER_DIR "/frag.spv")
            .setVertexInfo()
            .setPipelineLayout(layout) 
            .setColorBlending() 
            .setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .setViewport({800, 600})
            .setCullMode(VK_CULL_MODE_BACK_BIT)
            .setPolygonMode(VK_POLYGON_MODE_FILL)
            .setDynamicState(dynamicStates)
            .setMultisampling()
            .setRasterizer()
            .build();
    }
};