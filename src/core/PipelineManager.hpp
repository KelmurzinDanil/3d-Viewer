#pragma once
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
#include "BasicTriangleStrategy.hpp"
#include "BufferManager.hpp"
#include "Constants.hpp"

class PipelineManager {
    public:
        PipelineManager(DeviceManager& deviceMgr, SwapChainManager& swapMgr);
        
        void createPipelineLayout();
        void createGraphicsPipeline();

        void createDescriptorSetLayout();   
        void createDescriptorPool(); 
        void createDescriptorSets(const std::vector<VkBufferPtr>& uniformBuffers,
                                    VkSampler textureSampler,
                                    VkImageView textureImageView);

        VkPipelineLayout getLayout() const { return pipelineLayout_.get(); }
        VkPipeline getGraphicsPipeline() const { return graphicsPipeline_.get(); }
        std::vector<VkDescriptorSet> getDescriptorSets() const {return descriptorSets;}
    
    private:


        VkDescriptorSetLayoutPtr descriptorSetLayout;
        VkDescriptorPoolPtr descriptorPool;

        std::vector<VkDescriptorSet> descriptorSets;

        DeviceManager& deviceManager_;
        SwapChainManager& swapChainManager_;

        VkPipelineLayoutPtr pipelineLayout_;
        VkPipelinePtr graphicsPipeline_;
    };

