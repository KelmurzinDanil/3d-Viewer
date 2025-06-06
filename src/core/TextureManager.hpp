#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "VulkanUtils.hpp"
#include "BufferManager.hpp"
#include "DeviceManager.hpp"
#include "SwapChainManager.hpp"
class TextureManager{

    public:
    TextureManager(BufferManager& bufferManager, DeviceManager& deviceManager, SwapChainManager& swapChainManager);

    VkImageView getTextureImageView() const { return textureImageView.get(); }
    VkSampler getTextureSampler() const { return textureSampler.get(); }
    
    
        
    private:

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferManager& bufferManager_;
    DeviceManager& deviceManager_;
    SwapChainManager& swapChainManager_;

    VkImageViewPtr textureImageView;
    VkSamplerPtr textureSampler;

    VkImagePtr textureImage;
    VkDeviceMemoryPtr textureImageMemory;

   
    



    void createTextureSampler();
    void createTextureImageView();
    void createTextureImage();
};