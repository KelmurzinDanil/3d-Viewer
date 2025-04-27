#pragma once
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "BasicTriangleStrategy.hpp"

class PipelineManager {
    public:
        PipelineManager(DeviceManager& deviceMgr, SwapChainManager& swapMgr);
        
        void createPipelineLayout();
        void createGraphicsPipeline();
    
        VkPipelineLayout getLayout() const { return pipelineLayout_.get(); }
        VkPipeline getGraphicsPipeline() const { return graphicsPipeline_.get(); }
    
    private:
        DeviceManager& deviceManager_;
        SwapChainManager& swapChainManager_;
        
        VkPipelineLayoutPtr pipelineLayout_;
        VkPipelinePtr graphicsPipeline_;
    };

